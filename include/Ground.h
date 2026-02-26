#ifndef GROUND_H
#define GROUND_H

#include <memory>
#include <glm/glm.hpp>
#include "Shader.h"

class Ground
{
public:
    // 构造函数负责所有初始化：VAO/VBO生成、Shader编译、贴图加载
    Ground();
    ~Ground();

    // 每一帧只需要调这一个接口，传必要的环境参数进来
    void Draw(const glm::mat4& view, const glm::mat4& projection, float time,
        const glm::vec3& viewPos, const glm::vec3& lightPos,
        const glm::vec3& lightColor, float wetness);

private:
    std::unique_ptr<Shader> shader;

    unsigned int VAO, VBO;
    unsigned int diffMap, normMap, roughMap, aoMap, dispMap;

    void setupMesh();
    void loadTextures();
};

#endif