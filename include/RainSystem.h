#ifndef RAIN_SYSTEM_H
#define RAIN_SYSTEM_H

#include <vector>
#include <memory>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

class RainSystem
{
public:
    // 构造函数不再需要外部传入 Shader！只需要数量和贴图ID
    RainSystem(unsigned int amount, unsigned int textureID);

    void Update(float dt, glm::vec2 cameraPos, glm::vec3 windVelocity);

    void Draw(const glm::mat4& view, const glm::mat4& projection,
        const glm::vec3& cameraPos, float time,
        const glm::vec3& lightPos, const glm::vec3& lightColor,
        const glm::vec3& globalWind);

private:
    std::unique_ptr<Shader> shader; // [核心修改] 自己管理 Shader 智能指针
    unsigned int amount;
    unsigned int particleTexture;

    unsigned int VAO;
    unsigned int quadVBO;
    unsigned int instanceVBO;

    std::vector<glm::vec4> particleRenderData;
    std::vector<glm::vec3> particleVelocities;

    void init();
};

#endif