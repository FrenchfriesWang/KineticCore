#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <vector>
#include <glad/glad.h> // 关键：必须包含 glad 才能识别 OpenGL 类型
#include <glm/glm.hpp>

#include "Shader.h"
#include "Particle.h"

class ParticleSystem
{
public:
    // 构造函数
    ParticleSystem(Shader& shader, unsigned int amount);

    // 更新逻辑
    void Update(float dt, unsigned int newParticles, glm::vec2 offset);

    // 渲染逻辑
    void Draw();

private:
    // 粒子集合
    std::vector<Particle> particles;
    unsigned int amount;

    // Shader 引用
    // 使用引用(&)而不是拷贝，因为 Shader 通常包含 OpenGL ID，不宜拷贝
    Shader& shader;

    // --- 渲染管线核心变量 (修复报错的关键) ---
    unsigned int VAO;
    unsigned int quadVBO;     // 存储四边形顶点
    unsigned int instanceVBO; // 存储每个粒子的位置偏移

    // 初始化函数
    void init();
};

#endif