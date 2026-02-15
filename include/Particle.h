#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>

// 粒子个体
// 工业级考量：
// 1. 内存对齐：虽然现在是教学，但未来要注意结构体大小对缓存(Cache)的影响。
// 2. 构造函数初始化列表：性能最高。
struct Particle {
    glm::vec3 Position; // 位置
    glm::vec3 Velocity; // 速度
    glm::vec4 Color;    // 颜色

    float Life;         // 剩余寿命 (例如：1.0f 代表刚出生，<=0.0f 代表死亡)

    // 构造函数
    Particle()
        : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) {
    }
};

#endif