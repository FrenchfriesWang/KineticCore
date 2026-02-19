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
    // 1. 每一帧激活新的粒子
    for (unsigned int i = 0; i < newParticles; ++i)
    {
        int unusedParticle = firstUnusedParticle();
        Particle& p = particles[unusedParticle];

        // 如果粒子是未激活状态 (Life <= 0)，让它出生
        if (p.Life <= 0.0f)
        {
            // 生成在相机周围 x, z
            float rX = ((rand() % 200) - 100) / 5.0f; // -20 到 +20
            float rZ = ((rand() % 200) - 100) / 5.0f;
            float startY = 20.0f; // 高空生成

            p.Position = glm::vec3(offset.x + rX, startY, offset.y + rZ);
            p.Color = glm::vec4(0.8f, 0.85f, 1.0f, 1.0f); // 雨滴颜色

            // 只要赋值 > 0 即可，因为在 Falling 状态下我们要锁住它不减少
            p.Life = 1.0f;

            p.Velocity = glm::vec3(0.0f, -20.0f, 0.0f); // 高速下落
            p.State = ParticleState::Falling;
        }
    }

    // 2. 更新所有粒子
    for (unsigned int i = 0; i < amount; ++i)
    {
        Particle& p = particles[i];

        // 只处理活着的粒子
        if (p.Life > 0.0f)
        {
            // --- 状态：下落中 ---
            if (p.State == ParticleState::Falling)
            {
                // 更新物理位置
                p.Position += p.Velocity * dt;

                // 【核心修正】这里绝对不写 p.Life -= dt;
                // 让雨滴一直活到撞地为止

                // --- 高度检测 (触地逻辑) ---
                // 假设地面高度是 -2.0f
                if (p.Position.y < 0.0f)
                {
                    // 触地了，直接重置回天空 (无限循环)
                    float rX = ((rand() % 200) - 100) / 5.0f;
                    float rZ = ((rand() % 200) - 100) / 5.0f;

                    // 重新回到上方
                    p.Position = glm::vec3(offset.x + rX, 20.0f, offset.y + rZ);

                    // 保持 Life > 0，保持 Falling 状态
                }
            }
            // --- 状态：溅射中 (这是你未来做水花预留的) ---
            else if (p.State == ParticleState::Splashing)
            {
                // 只有水花才需要随时间消失
                p.Life -= dt;
                p.Color.a -= dt * 2.0f; // 快速变淡
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


unsigned int ParticleSystem::firstUnusedParticle()
{
    // 1. 从上次最后使用的位置开始向后搜索
    for (unsigned int i = lastUsedParticle; i < amount; ++i) {
        if (particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }

    // 2. 如果后面没找到，从头开始搜索
    for (unsigned int i = 0; i < lastUsedParticle; ++i) {
        if (particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }

    // 3. 如果所有粒子都活着，就覆盖第一个（或者覆盖第0个）
    lastUsedParticle = 0;
    return 0;
}