#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 相机移动方向的枚举
enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// 默认参数配置
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 1.8f; // [修改] 步伐调得极其沉重
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// --- 氛围向物理约束 ---
const float GROUND_HEIGHT = 1.5f; // 死锁的眼睛高度
const float BOUNDARY_LIMIT = 9.5f; // 地面边界 (因为地面是 10x10)

class Camera
{
public:
    // --- 核心属性 ---
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // 构造函数
    Camera(glm::vec3 position = glm::vec3(0.0f, GROUND_HEIGHT, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH);

    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(CameraMovement direction, float deltaTime);

    // 用来处理空气墙边界和死锁高度
    void UpdatePhysics(float deltaTime);

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

private:
    void updateCameraVectors();
};