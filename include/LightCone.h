#ifndef LIGHT_CONE_H
#define LIGHT_CONE_H

#include <memory>
#include <glm/glm.hpp>
#include "Shader.h"

class LightCone
{
public:
    LightCone();
    ~LightCone();

    // 绘制接口：内部会自动接管并恢复所有危险的 OpenGL 状态机
    void Draw(const glm::mat4& view, const glm::mat4& projection, float time, const glm::vec3& cameraPos);

private:
    std::unique_ptr<Shader> shader;
    unsigned int VAO, VBO;

    void setupMesh();
};

#endif