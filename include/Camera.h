#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 相机移动方向的枚举
enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// 默认参数配置 (工业级写法：集中管理常量)
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
    // --- 核心属性 ---
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // --- 欧拉角 ---
    float Yaw;
    float Pitch;

    // --- 选项 ---
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // 构造函数
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH);

    // 获取视图矩阵 (LookAt)
    glm::mat4 GetViewMatrix();

    // 处理键盘输入
    void ProcessKeyboard(CameraMovement direction, float deltaTime);

    // 处理鼠标移动
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // 处理鼠标滚轮 (缩放)
    void ProcessMouseScroll(float yoffset);

private:
    // 根据欧拉角计算 Front 向量
    void updateCameraVectors();
};