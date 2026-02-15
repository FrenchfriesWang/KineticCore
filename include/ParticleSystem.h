#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Particle.h"

class ParticleSystem
{
public:
    // 构造函数：告诉系统最多支持多少个粒子
    ParticleSystem(Shader shader, unsigned int amount);

    // 每帧调用：更新所有粒子的物理状态 (dt = deltaTime)
    // objectVelocity 用于实现“粒子继承物体速度”的效果（可选）
    // offset 用于新产生的粒子位置偏移
    void Update(float dt, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));

    // 每帧调用：渲染所有活着的粒子
    void Draw();

private:
    // --- 属性 ---
    std::vector<Particle> particles; // 粒子池 (Object Pool 思想，避免频繁 new/delete)
    unsigned int amount;             // 粒子总数上限

    // --- 渲染资源 ---
    Shader shader;
    unsigned int VAO;
    unsigned int textureID; // 暂时存一个纹理ID，未来可以用 ResourceManager

    // --- 内部工具 ---
    // 初始化粒子属性（当粒子死亡后复活时调用）
    void respawnParticle(Particle& particle, glm::vec2 offset = glm::vec2(0.0f, 0.0f));

    // 初始化 GPU 资源 (VAO/VBO)
    void initRenderData();

    unsigned int lastUsedParticle = 0;

    unsigned int firstUnusedParticle();
};

#endif