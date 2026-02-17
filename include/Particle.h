#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>

enum class ParticleState {
    Falling,
    Splashing
};

struct Particle {
    glm::vec3 Position; 
    glm::vec3 Velocity; 
    glm::vec4 Color;    
    float Life;         
    ParticleState State; 

    // 默认构造函数初始化
    Particle()
        : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) {
    }
};

#endif