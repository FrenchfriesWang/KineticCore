#include "Ground.h"
#include "TextureUtils.h"
#include <glad/glad.h>

Ground::Ground()
{
    // 1. 初始化 Shader
    shader = std::make_unique<Shader>("assets/shaders/ground.vert", "assets/shaders/ground.frag");

    // 2. 初始化网格和贴图
    setupMesh();
    loadTextures();

    // 3. 预绑定纹理单元 (只需做一次)
    shader->use();
    shader->setInt("albedoMap", 0);
    shader->setInt("normalMap", 1);
    shader->setInt("roughnessMap", 2);
    shader->setInt("aoMap", 3);
    shader->setInt("dispMap", 4);
}

Ground::~Ground()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &diffMap);
    glDeleteTextures(1, &normMap);
    glDeleteTextures(1, &roughMap);
    glDeleteTextures(1, &aoMap);
    glDeleteTextures(1, &dispMap);
}

void Ground::setupMesh()
{
    float planeVertices[] = {
        // positions            // normals         // texcoords
        -10.0f, 0.0f,  10.0f,   0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
         10.0f, 0.0f,  10.0f,   0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
         10.0f, 0.0f, -10.0f,   0.0f, 1.0f, 0.0f,  10.0f, 10.0f,

         10.0f, 0.0f, -10.0f,   0.0f, 1.0f, 0.0f,  10.0f, 10.0f,
        -10.0f, 0.0f, -10.0f,   0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
        -10.0f, 0.0f,  10.0f,   0.0f, 1.0f, 0.0f,   0.0f,  0.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void Ground::loadTextures()
{
    diffMap = loadTexture("assets/textures/cobblestone_ground_diff.jpg");
    normMap = loadTexture("assets/textures/cobblestone_ground_nor_gl.jpg");
    roughMap = loadTexture("assets/textures/cobblestone_ground_rough.jpg");
    aoMap = loadTexture("assets/textures/cobblestone_ground_ao.jpg");
    dispMap = loadTexture("assets/textures/cobblestone_ground_disp.jpg");
}

void Ground::Draw(const glm::mat4& view, const glm::mat4& projection, float time,
    const glm::vec3& viewPos, const glm::vec3& lightPos,
    const glm::vec3& lightColor, float wetness)
{
    shader->use();
    shader->setFloat("time", time);
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", glm::mat4(1.0f)); // 地面暂时不移动

    shader->setVec3("viewPos", viewPos);
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("lightColor", lightColor);
    shader->setFloat("wetness", wetness);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, diffMap);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, normMap);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, roughMap);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, aoMap);
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, dispMap);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}