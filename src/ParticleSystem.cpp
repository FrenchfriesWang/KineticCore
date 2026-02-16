#include "ParticleSystem.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

// 简单的随机数辅助函数
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
    // 1. 初始化粒子数据 (CPU)
    for (unsigned int i = 0; i < this->amount; ++i)
    {
        Particle p;
        // 初始位置：在空中随机分布 (X: -20~20, Y: 5~20, Z: -20~20)
        p.Position = glm::vec3(randomFloat(-20.0f, 20.0f), randomFloat(5.0f, 25.0f), randomFloat(-20.0f, 20.0f));
        p.Velocity = glm::vec3(0.0f, -5.0f, 0.0f); // 初始下落速度
        p.Color = glm::vec4(1.0f);
        p.Life = randomFloat(0.5f, 1.0f);
        particles.push_back(p);
    }

    // 2. 配置 OpenGL 缓冲 (GPU)

    // --- 基础四边形 (Quad) 顶点数据 ---
    // 只需要位置 (Location 0)，UV 在 Shader 里自动计算了
    float quadVertices[] = {
        // positions (中心点为 0,0)
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->quadVBO);
    glGenBuffers(1, &this->instanceVBO); // 关键：这是存位置偏移量的 Buffer

    glBindVertexArray(this->VAO);

    // 设置 Quad VBO
    glBindBuffer(GL_ARRAY_BUFFER, this->quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // 属性 0: 顶点位置 (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // 设置 Instance VBO (初始为空，大小预留)
    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
    // 预分配内存，GL_STREAM_DRAW 表示我们会频繁修改它
    glBufferData(GL_ARRAY_BUFFER, this->amount * sizeof(glm::vec3), NULL, GL_STREAM_DRAW);

    // 属性 2: 实例偏移 (vec3) -> 对应 Shader 里的 layout (location = 2) in vec3 offset;
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    // [核弹级代码] 这一行告诉 OpenGL：这个属性是每个 Instance 读一次，而不是每个 Vertex 读一次
    // 如果没有这一行，所有粒子都会叠在一起！
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
}

void ParticleSystem::Update(float dt, unsigned int newParticles, glm::vec2 offset)
{
    // 更新所有粒子
    for (unsigned int i = 0; i < this->amount; ++i)
    {
        Particle& p = this->particles[i];

        // 物理逻辑：简单的重力下落
        p.Velocity.y -= 9.8f * dt * 0.1f; // 稍微给点重力加速度
        p.Position += p.Velocity * dt;

        // 边界检查：如果掉到地面以下 (Y < -2.0)
        if (p.Position.y < -2.0f)
        {
            // 重置到天上
            p.Position = glm::vec3(randomFloat(-20.0f, 20.0f), randomFloat(10.0f, 25.0f), randomFloat(-20.0f, 20.0f));
            p.Velocity = glm::vec3(0.0f, randomFloat(-5.0f, -10.0f), 0.0f); // 随机初速度
        }
    }
}

void ParticleSystem::Draw()
{
    // 1. 收集这一帧所有粒子的位置
    // 我们需要把 std::vector<Particle> 里的 Position 提取到一个连续的数组里传给 GPU
    std::vector<glm::vec3> positionData;
    positionData.reserve(this->amount);

    for (const auto& p : this->particles)
    {
        positionData.push_back(p.Position);
    }

    // 2. 更新 GPU 显存
    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
    // 使用 glBufferSubData 或者 glBufferData (orphaning) 更新数据
    // 这里直接覆盖整个 Buffer，虽然不是极致优化，但最稳
    glBufferData(GL_ARRAY_BUFFER, positionData.size() * sizeof(glm::vec3), positionData.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 3. 绘制调用
    this->shader.use();
    glBindVertexArray(this->VAO);
    // 使用 Instanced 绘制：画 positionData.size() 个四边形
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(positionData.size()));
    glBindVertexArray(0);
}