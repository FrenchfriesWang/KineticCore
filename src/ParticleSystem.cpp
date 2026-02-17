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
        p.Velocity = glm::vec3(0.0f, randomFloat(-10.0f, -20.0f), 0.0f);
        p.Color = glm::vec4(1.0f);
        p.Life = 1.0f;
        p.State = ParticleState::Falling;
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
    glGenBuffers(1, &this->instanceVBO); // 这是存位置偏移量的 Buffer

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
    glBufferData(GL_ARRAY_BUFFER, this->amount * sizeof(glm::vec4), NULL, GL_STREAM_DRAW);

    // 属性 2: 实例偏移 (vec3) -> 对应 Shader 里的 layout (location = 2) in vec3 offset;
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);

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

        if (p.State == ParticleState::Falling)
        {
            // 重力加速
            p.Velocity.y -= 9.8f * dt;
            p.Position += p.Velocity * dt;

            // 撞地检测 (假设地面在 Y = 0)
            if (p.Position.y <= 0.0f)
            {
                p.Position.y = 0.05f; // 稍微浮起一点，避免Z-fighting
                p.State = ParticleState::Splashing;
                p.Velocity = glm::vec3(0.0f); // 停在地面
                p.Life = 0.15f; // 涟漪只存活 0.15 秒
            }
        }
        else if (p.State == ParticleState::Splashing)
        {
            // 涟漪逻辑：倒计时
            p.Life -= dt;
            if (p.Life <= 0.0f)
            {
                // 重生回天空
                p.State = ParticleState::Falling;
                p.Position = glm::vec3(randomFloat(-20.0f, 20.0f), randomFloat(10.0f, 30.0f), randomFloat(-20.0f, 20.0f));
                p.Velocity = glm::vec3(0.0f, randomFloat(-10.0f, -20.0f), 0.0f);
            }
        }
    }
}

void ParticleSystem::Draw()
{
    std::vector<glm::vec4> instanceData;
    instanceData.reserve(this->amount);

    for (const auto& p : this->particles)
    {
        float wComponent = 1.0f;

        if (p.State == ParticleState::Falling)
        {
            // 雨滴状态：w > 0，代表垂直拉伸系数
            wComponent = 1.0f;
        }
        else if (p.State == ParticleState::Splashing)
        {
            // 水花状态：w < 0，我们要传回当前的 Life
            // 加负号是为了让 Shader 知道它是水花
            // 比如 Life 是 0.15，传过去就是 -0.15
            // 避免 -0.0 (虽然浮点数有 -0，但为了安全我们给个极小偏移)
            wComponent = -p.Life;
            if (wComponent == 0.0f) wComponent = -0.0001f;
        }

        instanceData.push_back(glm::vec4(p.Position, wComponent));
    }

    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(glm::vec4), instanceData.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    this->shader.use();
    glBindVertexArray(this->VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(instanceData.size()));
    glBindVertexArray(0);
}