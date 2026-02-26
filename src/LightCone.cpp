#include "LightCone.h"
#include <glad/glad.h>
#include <vector>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

LightCone::LightCone()
{
    // 严格对应小写蛇形命名的资产文件
    shader = std::make_unique<Shader>("assets/shaders/light_cone.vert", "assets/shaders/light_cone.frag");
    setupMesh();
}

LightCone::~LightCone()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void LightCone::setupMesh()
{
    std::vector<float> vertices;
    const int segments = 72;
    const float bottomRadius = 3.8f;
    const float topRadius = 0.3f;
    const float height = -9.0f;

    glm::vec3 topCenter(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < segments; ++i)
    {
        float angle1 = ((float)i / segments) * 2.0f * 3.1415926535f;
        float angle2 = ((float)(i + 1) / segments) * 2.0f * 3.1415926535f;

        glm::vec3 b1(cos(angle1) * bottomRadius, height, sin(angle1) * bottomRadius);
        glm::vec3 b2(cos(angle2) * bottomRadius, height, sin(angle2) * bottomRadius);
        glm::vec3 t1(cos(angle1) * topRadius, 0.0f, sin(angle1) * topRadius);
        glm::vec3 t2(cos(angle2) * topRadius, 0.0f, sin(angle2) * topRadius);

        // 侧面
        vertices.push_back(t1.x); vertices.push_back(t1.y); vertices.push_back(t1.z);
        vertices.push_back(b2.x); vertices.push_back(b2.y); vertices.push_back(b2.z);
        vertices.push_back(b1.x); vertices.push_back(b1.y); vertices.push_back(b1.z);

        vertices.push_back(t1.x); vertices.push_back(t1.y); vertices.push_back(t1.z);
        vertices.push_back(t2.x); vertices.push_back(t2.y); vertices.push_back(t2.z);
        vertices.push_back(b2.x); vertices.push_back(b2.y); vertices.push_back(b2.z);

        // 顶盖 (封死黑洞)
        vertices.push_back(topCenter.x); vertices.push_back(topCenter.y); vertices.push_back(topCenter.z);
        vertices.push_back(t1.x); vertices.push_back(t1.y); vertices.push_back(t1.z);
        vertices.push_back(t2.x); vertices.push_back(t2.y); vertices.push_back(t2.z);

        vertices.push_back(topCenter.x); vertices.push_back(topCenter.y); vertices.push_back(topCenter.z);
        vertices.push_back(t2.x); vertices.push_back(t2.y); vertices.push_back(t2.z);
        vertices.push_back(t1.x); vertices.push_back(t1.y); vertices.push_back(t1.z);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void LightCone::Draw(const glm::mat4& view, const glm::mat4& projection, float time, const glm::vec3& cameraPos)
{
    // --- 状态机隔离开始 ---
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setFloat("time", time);

    glm::mat4 coneModel = glm::mat4(1.0f);
    coneModel = glm::translate(coneModel, glm::vec3(0.0f, 9.0f, 0.0f));
    shader->setMat4("model", coneModel);

    shader->setVec3("lightPos", glm::vec3(0.0f, 9.0f, 0.0f));
    shader->setVec3("lightColor", glm::vec3(0.8f, 0.9f, 1.0f) * 1.5f);
    shader->setVec3("cameraPos", cameraPos);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 72 * 12);
    glBindVertexArray(0);

    // --- 状态机隔离结束，绝对恢复原状 ---
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
}