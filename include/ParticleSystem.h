#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <vector>
#include <memory>
#include <glad/glad.h>
#include <glm/glm.hpp>


#include "Shader.h"
#include "Particle.h"

class ParticleSystem
{
public:
    ParticleSystem(Shader& shader, unsigned int amount); // 注意：shader 改为传引用，避免拷贝
    ~ParticleSystem();

    //// 每帧调用：更新所有粒子的物理状态 (dt = deltaTime)
    //// objectVelocity 用于实现“粒子继承物体速度”的效果（可选）
    //// offset 用于新产生的粒子位置偏移
    //void Update(float dt, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));

    //// 每帧调用：渲染所有活着的粒子
    //void Draw();
    void Update(float dt, unsigned int newParticles, glm::vec2 offset);
    void Draw();

private:
    // --- 属性 ---
    std::vector<Particle> particles; // 粒子池 (Object Pool 思想，避免频繁 new/delete)
    unsigned int amount;             // 粒子总数上限

    // --- 渲染资源 ---
    Shader& shader;

   // 管理渲染状态
    unsigned int quadVAO;

   // 内部初始化函数
    void init();

    // 查找未使用粒子的索引
    unsigned int firstUnusedParticle();
    void respawnParticle(Particle& particle, glm::vec2 offset);
};

#endif