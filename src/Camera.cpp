#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVITY),
    Zoom(ZOOM)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;

    // 依然保持只在 XZ 水平面平移
    glm::vec3 flatFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
    glm::vec3 flatRight = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

    if (direction == CameraMovement::FORWARD)
        Position += flatFront * velocity;
    if (direction == CameraMovement::BACKWARD)
        Position -= flatFront * velocity;
    if (direction == CameraMovement::LEFT)
        Position -= flatRight * velocity;
    if (direction == CameraMovement::RIGHT)
        Position += flatRight * velocity;

    // [核心修改]：彻底删除 UP 和 DOWN 键的逻辑，没有任何跳跃和下蹲！
}

// [核心修改]：变成纯粹的空气墙和高度死锁管理器
void Camera::UpdatePhysics(float deltaTime)
{
    // 1. 沉重的肉身：永远锁死在 1.7 米，杜绝飞天遁地
    Position.y = GROUND_HEIGHT;

    // 2. 空气墙边界拦截：走出边界就强行拉回来
    if (Position.x > BOUNDARY_LIMIT) Position.x = BOUNDARY_LIMIT;
    if (Position.x < -BOUNDARY_LIMIT) Position.x = -BOUNDARY_LIMIT;
    if (Position.z > BOUNDARY_LIMIT) Position.z = BOUNDARY_LIMIT;
    if (Position.z < -BOUNDARY_LIMIT) Position.z = -BOUNDARY_LIMIT;
}

// ... 鼠标移动、滚轮和 updateCameraVectors 保持原样不动 ...
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity; yoffset *= MouseSensitivity;
    Yaw += xoffset; Pitch += yoffset;
    if (constrainPitch) {
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f) Zoom = 1.0f;
    if (Zoom > 45.0f) Zoom = 45.0f;
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}