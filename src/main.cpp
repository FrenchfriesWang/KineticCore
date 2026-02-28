#include <iostream>
#include <vector>
#include <memory>



#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "RainSystem.h"

#include <cmath>

#include "TextureUtils.h"
#include "LightCone.h"
#include "Ground.h"
#include "AudioSystem.h"
// 函数声明
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
bool processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


// 屏幕设置
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// 相机实例
Camera camera(glm::vec3(0.0f, 1.8f, 6.7f), glm::vec3(0.0f, 1.0f, 0.0f), YAW, -40.0f);

// 鼠标控制相关
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 时间差处理 
float deltaTime = 0.0f;
float lastFrame = 0.0f;



int main()
{
	// ------------------------------
	// 1. 初始化 GLFW
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "KineticCore - Refactored Shader", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 绑定鼠标回调
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// 捕获鼠标，且隐藏光标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// ------------------------------
	// 2. 初始化 GLAD
	// ------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// ------------------------------
	// 3. 配置全局 OpenGL 状态
    // ------------------------------
	// 开启深度测试 (3D 渲染必须开启，否则后面的面会挡住前面的面)
	glEnable(GL_DEPTH_TEST);


	// 混合模式调整：从 GL_ONE (叠加发光) 改为 标准 Alpha 混合
	// 让雨滴看起来更像水，而不是发光的激光束
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// ------------------------------
	// 4. 初始化资源 (使用智能指针)
	// ------------------------------

    // 使用 std::unique_ptr 管理 Shader
	unsigned int rainTextureID = generateRaindropTexture();
	auto rainSystem = std::make_unique<RainSystem>(25000, rainTextureID);

	auto ground = std::make_unique<Ground>();
	auto lightCone = std::make_unique<LightCone>();

	// 初始化音频系统并播放雨声
	auto audioSystem = std::make_unique<AudioSystem>();
	audioSystem->PlayRain();

	// 记录上一次真实发出脚步声的物理 XZ 坐标 (锚点)
	glm::vec2 lastFootprintPos(camera.Position.x, camera.Position.z);
	glm::vec3 accumulatedWindOffset(0.0f);

	// ------------------------------
	// 5. 渲染循环
	// ------------------------------
	while (!glfwWindowShouldClose(window))
	{
		// 计算 DeltaTime 
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// --- 核心修改区：计算真实位移并修复两次输入的 Bug ---

		glm::vec3 oldPos = camera.Position;
		processInput(window);

		camera.UpdatePhysics(deltaTime);

		// 计算物理移动真实距离
		float distanceMoved = glm::distance(glm::vec2(oldPos.x, oldPos.z), glm::vec2(camera.Position.x, camera.Position.z));

		// 让 Camera 计算起伏。如果算到刚刚好踩到底部，就会返回 true
		bool stepTriggered = camera.UpdateHeadBob(distanceMoved, deltaTime);

		if (stepTriggered) {
			glm::vec2 currentXZ(camera.Position.x, camera.Position.z);

			// 算一下：当前坐标离上一次发声的脚印，直线距离有没有超过 0.3 米？
			// 0.3 米是一个非常安全的防抖阈值（正常一步大概 0.85 米，而 ADAD 抽搐通常在 0.1 米内）
			if (glm::distance(currentXZ, lastFootprintPos) > 0.3f) {

				// 根据当前的摄像机速度，判断是走路还是跑步
				// (你可以根据你的需求微调这个 2.5f 的界限值)
				bool isRunning = (camera.MovementSpeed > 2.5f);

				audioSystem->PlayFootstep(isRunning);

				// 声音成功发出，把锚点更新到当前脚下
				lastFootprintPos = currentXZ;
			}
			// 如果没超过 0.3 米，就什么也不播，完美过滤掉原地抖动！
		}

		// 清屏 (背景色设为深色，接近纯黑的虚空感)
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --- 1. 统一计算矩阵 (供所有 Shader 使用) ---
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		// --- 2. 渲染地面 (PBR Wetness) ---
		ground->Draw(view, projection, currentFrame, camera.Position, glm::vec3(0.0f, 9.0f, 0.0f), glm::vec3(0.8f, 0.9f, 1.0f) * 4.5f, 0.45f);

		// --- 3. 渲染粒子 (包含动态微风计算) ---
		float windTime = currentFrame * 0.4f;
		float windX = sin(windTime * 0.5f) * 3.5f + cos(windTime * 0.2f) * 0.8f;
		float windZ = cos(windTime * 0.3f) * 1.0f;
		glm::vec3 currentWindVelocity = glm::vec3(windX, 0.0f, windZ);
		accumulatedWindOffset += currentWindVelocity * deltaTime;

		rainSystem->Draw(view, projection, camera.Position, currentFrame,
			glm::vec3(0.0f, 9.0f, 0.0f), glm::vec3(0.8f, 0.9f, 1.0f) * 4.5f,
			accumulatedWindOffset, currentWindVelocity);

		// --- 4. 渲染体积光锥 (Additive Blending) ---
		lightCone->Draw(view, projection, currentFrame, camera.Position);

		// 交换缓冲 & 轮询事件
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ------------------------------
	// 6. 资源释放
	// ------------------------------
	// unique_ptr 会在这里自动释放 particleSystem 和 shader，无需 delete
	// 只需处理 OpenGL 的资源
	glDeleteTextures(1, &rainTextureID);
	glfwTerminate();
	return 0;
}

bool processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Shift 奔跑逻辑：动态修改相机的移动速度
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.MovementSpeed = RUN_SPEED; // 奔跑速度 (大于我们设定的 2.5f 阈值，会自动触发 runSounds)
	}
	else {
		camera.MovementSpeed = SPEED; // 正常走路速度
	}

	// 收集输入方向
	glm::vec2 inputDir(0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) inputDir.y += 1.0f; // 前
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) inputDir.y -= 1.0f; // 后
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) inputDir.x -= 1.0f; // 左
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) inputDir.x += 1.0f; // 右

	bool isMoving = (inputDir.x != 0.0f || inputDir.y != 0.0f);

	if (isMoving) {
		// [核心修复] 归一化输入向量，防止斜向移动速度变为 1.414 倍
		inputDir = glm::normalize(inputDir);

		// 手动拆分并调用，保持与原本 Camera 的接口兼容
		// 注意：这里的 deltaTime 依然在内部乘了 MovementSpeed
		if (inputDir.y > 0.0f) camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime * inputDir.y);
		if (inputDir.y < 0.0f) camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime * -inputDir.y);
		if (inputDir.x < 0.0f) camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime * -inputDir.x);
		if (inputDir.x > 0.0f) camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime * inputDir.x);
	}

	return isMoving;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}