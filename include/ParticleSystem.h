#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

class ParticleSystem
{
public:
    ParticleSystem(Shader& shader, unsigned int amount);

    // 只需要传入 delta time 和相机的 XZ 坐标
    void Update(float dt, glm::vec2 cameraPos);
    void Draw(glm::vec3 cameraPos);

private:
    Shader& shader;
    unsigned int amount;

    unsigned int VAO;
    unsigned int quadVBO;
    unsigned int instanceVBO;

    // --- [核心优化：面向数据设计 SoA] ---
    // 抛弃庞大的 Particle 结构体，将 GPU 需要的数据和 CPU 需要的数据彻底分离
    // 这让 CPU 的 L1 Cache 能一次性塞入成百上千个数据，极限压榨硬件性能

    // 1. GPU 渲染数据：xyz = 世界坐标，w = 雨滴的随机缩放值 (决定粗细大小)
    // 这个数组的内存布局和 OpenGL 需要的完全一致，Draw 时直接强转指针提交，0 拷贝！
    std::vector<glm::vec4> particleRenderData;

    // 2. CPU 物理数据：仅用于 Update 计算位置
    std::vector<glm::vec3> particleVelocities;

    void init();
};

#endif