#include "ParticleSystem.h"
#include <iostream>

// 辅助函数
float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

ParticleSystem::ParticleSystem(Shader& shader, unsigned int amount)
    : shader(shader), amount(amount)
{
    this->init();
}

void ParticleSystem::init()
{
    // --- 1. 初始化 DOD 数组 ---
    particleRenderData.resize(amount);
    particleVelocities.resize(amount);

    for (unsigned int i = 0; i < amount; ++i)
    {
        // [修改] 范围稍微缩小一点，让雨水更集中在玩家周围
        float x = randomFloat(-20.0f, 20.0f);
        // [修改] 高度范围从 0~40 压缩回 10~30，落差 20 米，提高垂直密度
        // 这里的基准高度 raised 到 10.0f 以上，避免刚生成就可能在地面以下
        float y = randomFloat(10.0f, 30.0f);
        float z = randomFloat(-20.0f, 20.0f);

        float randomScale = randomFloat(0.5f, 1.5f);
        particleRenderData[i] = glm::vec4(x, y, z, randomScale);

        // [修改] 稍微加快一点下落速度，增加暴雨感
        particleVelocities[i] = glm::vec3(0.0f, randomFloat(-30.0f, -45.0f), 0.0f);
    }

    // --- 2. 配置 OpenGL ---
    float quadVertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->quadVBO);
    glGenBuffers(1, &this->instanceVBO);

    glBindVertexArray(this->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
    // [关键] 预分配显存，使用 GL_DYNAMIC_DRAW 因为每一帧都会更新
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
}

void ParticleSystem::Update(float dt, glm::vec2 cameraPos, glm::vec3 windVelocity)
{
    for (unsigned int i = 0; i < amount; ++i)
    {
        // 1. 物理更新：基础垂直速度 + 外部风力推挤
        particleRenderData[i].x += (particleVelocities[i].x + windVelocity.x) * dt;
        particleRenderData[i].y += particleVelocities[i].y * dt;
        particleRenderData[i].z += (particleVelocities[i].z + windVelocity.z) * dt;

        // 2. 无脑触地检测
        if (particleRenderData[i].y < -2.0f)
        {
            // [修复] 绝对随机的重生高度，彻底打碎虚线感！(不要加 cameraPos.y，那是 Z 坐标！)
            particleRenderData[i].y = 40.0f + randomFloat(0.0f, 15.0f);

            // [修复] vec2 只有 x 和 y！这里的 cameraPos.y 实际上就是 3D 世界的 Z！
            particleRenderData[i].x = cameraPos.x + randomFloat(-30.0f, 30.0f);
            particleRenderData[i].z = cameraPos.y + randomFloat(-30.0f, 30.0f);

            particleVelocities[i].y = randomFloat(-30.0f, -45.0f);
        }
    }
}

void ParticleSystem::Draw(glm::vec3 cameraPos)
{
    // --- [核弹级优化 2] 零拷贝直接提交数据 ---
    // 不再每一帧 new 和 delete vector。
    // 因为 particleRenderData 的底层内存布局就是紧凑的 vec4 数组，直接传指针给 GPU！
    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);

    // 使用 glBufferSubData 仅替换数据，不重新分配内存
    glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::vec4), particleRenderData.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    this->shader.use();
    this->shader.setVec3("cameraPos", cameraPos);

    glBindVertexArray(this->VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, amount);
    glBindVertexArray(0);
}