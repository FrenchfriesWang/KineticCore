#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>

struct Particle {
    glm::vec3 Position; // 位置
    glm::vec3 Velocity; // 速度
    glm::vec4 Color;    // 颜色
    float Life;         // 剩余寿命 (用于状态机或渐变)

    // 默认构造函数初始化
    Particle()
        : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) {
    }
};

#endif