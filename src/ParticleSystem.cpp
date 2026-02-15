#include "ParticleSystem.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random> // C++11 随机数库

// 构造函数
ParticleSystem::ParticleSystem(Shader& shader, unsigned int amount)
    : shader(shader), amount(amount)
{
    this->init();
}

// 析构函数 (别忘了清理 VAO)
ParticleSystem::~ParticleSystem()
{
    glDeleteVertexArrays(1, &this->quadVAO);
}

void ParticleSystem::Update(float dt, unsigned int newParticles, glm::vec2 offset)
{
    // 添加新粒子
    for (unsigned int i = 0; i < newParticles; ++i)
    {
        int unusedParticle = this->firstUnusedParticle();
        this->respawnParticle(this->particles[unusedParticle], offset);
    }

    // 更新所有粒子状态 (纯数据计算，不涉及 OpenGL)
    for (unsigned int i = 0; i < this->amount; ++i)
    {
        Particle& p = this->particles[i];
        p.Life -= dt;
        if (p.Life > 0.0f)
        {
            // [修改] 3D 物理运算
            p.Position -= p.Velocity * dt;
            p.Color.a -= dt * 2.5f;
        }
    }
}

void ParticleSystem::Draw()
{
    // 1. 设置混合模式 (为了防止粒子之间会有黑色方块遮挡，使用加法混合会更好看，或者按深度排序)
    // 但既然我们要 void rain 风格，先用普通的混合
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 暂时试一下 Additive，如果太亮再改回 ONE_MINUS_SRC_ALPHA

    this->shader.use();

    // [关键] 绑定我们在 init 里定义的 VAO
    glBindVertexArray(this->quadVAO);

    for (const Particle& p : this->particles)
    {
        if (p.Life > 0.0f)
        {
            // [修改] setVec2 -> setVec3
            this->shader.setVec3("offset", p.Position);
            this->shader.setVec4("color", p.Color);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }

    // 恢复默认混合模式 (可选)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleSystem::init()
{
    // 1. 初始化粒子池
    this->particles.resize(this->amount); // 使用 resize 预分配对象

    // 2. [关键] 把原本 main.cpp 里的顶点定义搬到这里！
    // -------------------------------------------------------
    float vertices[] = {
        // 位置(x,y,z)      // 纹理坐标(u,v)
        -0.05f, -0.05f, 0.0f,   0.0f, 0.0f,
         0.05f, -0.05f, 0.0f,   1.0f, 0.0f,
         0.05f,  0.05f, 0.0f,   1.0f, 1.0f,
        -0.05f,  0.05f, 0.0f,   0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int VBO, EBO;
    glGenVertexArrays(1, &this->quadVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(this->quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 属性 0: 位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 属性 1: 纹理坐标
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // 解绑

    // 清理 VBO/EBO (VAO 已经记录了状态，VBO 可以删掉或者留着，这里简单处理不删也行，因为类销毁时不管它们)
    // -------------------------------------------------------
}

// 查找算法：Last Used Optimization
unsigned int ParticleSystem::firstUnusedParticle()
{
    // 这里简单遍历，你可以加上 lastUsedParticle 优化
    for (unsigned int i = 0; i < this->amount; ++i) {
        if (this->particles[i].Life <= 0.0f) {
            return i;
        }
    }
    return 0; // 这里的覆盖逻辑可以优化
}

void ParticleSystem::respawnParticle(Particle& particle, glm::vec2 offset)
{
    // 随机生成 X 和 Z 轴的偏移 (水平面)
    float randomX = ((rand() % 100) - 50) / 10.0f;
    float randomZ = ((rand() % 100) - 50) / 10.0f;
    float rColor = 0.5f + ((rand() % 100) / 100.0f);

    // [关键修复]
    // 1. 我们把 2D 的 offset (玩家位置) 映射到 3D 空间
    //    玩家的 x -> 世界的 x
    //    玩家的 y -> 世界的 z (因为在 OpenGL 中xz平面是地平)

    // 2. Y 轴固定在空中 (比如 5.0f 高空)
    particle.Position = glm::vec3(randomX + offset.x, 5.0f, randomZ + offset.y);

    particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle.Life = 1.0f;

    // [关键修复] 速度也是 3D 的，雨是向下落 (Y轴负方向)
    particle.Velocity = glm::vec3(0.0f, 5.0f, 0.0f); // 这里的数值根据你的dt和单位可能需要调整
}