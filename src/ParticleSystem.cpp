#include "ParticleSystem.h"

#include <random> // C++11 随机数库

// 构造函数
ParticleSystem::ParticleSystem(Shader shader, unsigned int amount)
    : shader(shader), amount(amount), lastUsedParticle(0)
{
    this->initRenderData();

    // 预分配内存（对象池核心）
    // reserve 不会创建对象，只是申请内存；resize 会创建对象
    this->particles.resize(amount);
}

void ParticleSystem::Update(float dt, unsigned int newParticles, glm::vec2 offset)
{
    // 1. 产生新雨滴
    for (unsigned int i = 0; i < newParticles; ++i)
    {
        int unusedParticle = this->firstUnusedParticle();
        this->respawnParticle(this->particles[unusedParticle], offset);
    }

    // 2. 更新雨滴
    for (unsigned int i = 0; i < this->amount; ++i)
    {
        Particle& p = this->particles[i];
        if (p.Life > 0.0f)
        {
            p.Position += p.Velocity * dt;

            // --- 循环机制 ---
            // 假设地面是 Y = 0.0
            // 如果掉到地下 (Y < -0.5)，直接瞬移回天空，不用 delete
            if (p.Position.y < -0.5f)
            {
                this->respawnParticle(p, offset);
            }
        }
    }
}

void ParticleSystem::Draw()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 加法混合让雨滴发亮
    glDepthMask(GL_FALSE);

    this->shader.use();

    for (Particle particle : this->particles)
    {
        if (particle.Life > 0.0f)
        {
            // ==========================
            // 1. 绘制真身 (Real Rain)
            // ==========================
            this->shader.setVec4("color", particle.Color);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, particle.Position);
            this->shader.setMat4("model", model);

            glBindVertexArray(this->VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // ==========================
            // 2. 绘制倒影 (Reflection)
            // ==========================
            // 只有当雨滴接近地面时才画倒影(可选优化)，或者全部画
            // 这里为了简单，全部画，但把颜色调暗

            // 倒影颜色：亮度减半，模拟地面吸收
            this->shader.setVec4("color", particle.Color * 0.3f);

            glm::mat4 reflectModel = glm::mat4(1.0f);

            // 核心数学：镜像位置 (x, -y, z)
            // 假设地面是 Y=0。如果雨滴在 Y=5，倒影就在 Y=-5
            glm::vec3 reflectPos = particle.Position;
            reflectPos.y = -reflectPos.y;

            reflectModel = glm::translate(reflectModel, reflectPos);
            // 这一步可选：如果雨滴纹理有方向（比如上面尖下面圆），倒影也需要上下翻转
            // reflectModel = glm::scale(reflectModel, glm::vec3(1, -1, 1)); 

            this->shader.setMat4("model", reflectModel);

            // 复用同一个 VAO 绘制
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleSystem::initRenderData()
{
    // 配置雨滴模型 (Quad)
        // 形状：极细(0.015)、极长(0.3) -> 形成线条感
    float vertices[] = {
        // Pos                 // UV
        -0.015f, -0.3f, 0.0f,   0.0f, 0.0f,
         0.015f, -0.3f, 0.0f,   1.0f, 0.0f,
         0.015f,  0.3f, 0.0f,   1.0f, 1.0f,
        -0.015f,  0.3f, 0.0f,   0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int VBO, EBO;
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(this->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 属性 0: Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 属性 1: UV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// 工业级查找算法：Last Used Optimization
unsigned int ParticleSystem::firstUnusedParticle()
{
    // 1. 从上次最后一次使用的位置往后找
    for (unsigned int i = lastUsedParticle; i < amount; ++i) {
        if (particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    // 2. 如果后面满了，从头开始找 (线性查找)
    for (unsigned int i = 0; i < lastUsedParticle; ++i) {
        if (particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    // 3. 全满了：覆盖第一个（或者直接不生成，这里选择覆盖第0个保证即使满了也有新粒子）
    lastUsedParticle = 0;
    return 0;
}

void ParticleSystem::respawnParticle(Particle& particle, glm::vec2 offset)
{
    //// 使用 std::random_device 和 std::mt19937 生成高质量随机数
    //static std::random_device rd;
    //static std::mt19937 gen(rd());
    //static std::uniform_real_distribution<float> randomPosOffset(-0.5f, 0.5f);
    //static std::uniform_real_distribution<float> randomColor(0.5f, 1.0f);
    //static std::uniform_real_distribution<float> randomVelocity(-0.5f, 0.5f);

    //// 随机偏移位置
    //float rColor = 0.5f + ((rand() % 100) / 100.0f);

    //// 重置粒子属性
    //particle.Position = glm::vec3(offset.x + randomPosOffset(gen), offset.y + randomPosOffset(gen), offset.y + randomPosOffset(gen)); // 这里简单处理，稍后根据需求调整 Z 轴

    //// 让颜色稍微随机一点
    //particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);

    //particle.Life = 1.0f; // 活 1.0 单位时间

    //// 给个向上的初速度，加上一点随机扰动
    //particle.Velocity = glm::vec3(randomVelocity(gen), 1.5f, randomVelocity(gen));


    static std::random_device rd;
    static std::mt19937 gen(rd());

    // 范围：覆盖 XZ 平面 (-15 到 15)，高度 Y 固定在 15 (天空)
    static std::uniform_real_distribution<float> randomX(-15.0f, 15.0f);
    static std::uniform_real_distribution<float> randomZ(-15.0f, 5.0f); // 稍微偏向相机前方
    static std::uniform_real_distribution<float> randomY(15.0f, 25.0f); // 初始在很高的地方

    particle.Position = glm::vec3(randomX(gen), randomY(gen), randomZ(gen));

    // 颜色：半透明的白/蓝 (Alpha 0.5)
    particle.Color = glm::vec4(0.8f, 0.85f, 1.0f, 0.5f);

    particle.Life = 999.0f; // 不死之身，手动循环

    // 速度：极快向下 (-18 到 -12)
    static std::uniform_real_distribution<float> randomSpeedY(-18.0f, -12.0f);
    particle.Velocity = glm::vec3(0.0f, randomSpeedY(gen), 0.0f);
}