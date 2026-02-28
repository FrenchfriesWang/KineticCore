#include "RainSystem.h"
#include <iostream>
#include <cstdlib>

static float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

// [核心修改] 构造函数内部自己初始化 Shader！
RainSystem::RainSystem(unsigned int amount, unsigned int textureID)
    : amount(amount), particleTexture(textureID)
{
    shader = std::make_unique<Shader>("assets/shaders/rain.vert", "assets/shaders/rain.frag");
    this->init();
}

void RainSystem::init()
{
    particleRenderData.resize(amount);
    particleVelocities.resize(amount);

    for (unsigned int i = 0; i < amount; ++i)
    {
        float x = randomFloat(-30.0f, 30.0f);
        float y = randomFloat(10.0f, 40.0f);
        float z = randomFloat(-30.0f, 30.0f);
        float randomScale = randomFloat(0.5f, 1.5f);

        particleRenderData[i] = glm::vec4(x, y, z, randomScale);
        particleVelocities[i] = glm::vec3(0.0f, randomFloat(-30.0f, -45.0f), 0.0f);
    }

    float quadVertices[] = {
        -0.5f, -0.5f, 0.0f,  0.5f, -0.5f, 0.0f, -0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,  0.5f, -0.5f, 0.0f,  0.5f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &this->VAO); glGenBuffers(1, &this->quadVBO); glGenBuffers(1, &this->instanceVBO);
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::vec4), particleRenderData.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    glVertexAttribDivisor(2, 1);
    glBindVertexArray(0);
}


void RainSystem::Draw(const glm::mat4& view, const glm::mat4& projection,
    const glm::vec3& cameraPos, float time,
    const glm::vec3& lightPos, const glm::vec3& lightColor,
    const glm::vec3& windOffset, const glm::vec3& windVelocity)
{

    glDepthMask(GL_FALSE);

    // [核心修改] 从 this->shader.use() 变成了 this->shader->use()
    this->shader->use();
    this->shader->setInt("particleTexture", 0);
    this->shader->setMat4("projection", projection);
    this->shader->setMat4("view", view);
    this->shader->setFloat("time", time);
    this->shader->setVec3("cameraPos", cameraPos);
    this->shader->setVec3("windOffset", windOffset);
    this->shader->setVec3("windVelocity", windVelocity);

    this->shader->setVec3("lightPos", lightPos);
    this->shader->setFloat("coneHeight", 9.0f);
    this->shader->setFloat("coneBottomRadius", 3.8f);
    this->shader->setFloat("coneTopRadius", 0.3f);
    this->shader->setVec3("lightColor", lightColor);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, particleTexture);

    glBindVertexArray(this->VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, amount);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
}