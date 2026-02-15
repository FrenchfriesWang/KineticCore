#include <iostream>
#include <cmath>
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "ParticleSystem.h"

// STB_IMAGE_IMPLEMENTATION 宏会让库将实现代码编译进这个 cpp 文件
// 通常在大型项目中，会专门建立一个 src/stb_impl.cpp 来放这个宏，以加快编译速度
// 这里为了单文件连贯性，暂且放在 main.cpp 顶部
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// 全局变量配置
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 相机实例
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// 鼠标控制相关
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 时间差处理 
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int createSoftTexture();

int main()
{
	// --- 初始化 GLFW (保持不变) ---
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

	// 隐藏光标并捕捉 (FPS 游戏模式)
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// 初始化 GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 3. 开启深度测试 (3D 渲染必须开启，否则后面的面会挡住前面的面)
	glEnable(GL_DEPTH_TEST);

	// 构建 Shader
	// 自动读取文件、编译、链接、检错。
	// 注意路径：这里是相对于 VS "工作目录" (D:\Dev\KineticCore) 的路径
	Shader ourShader("assets/shaders/particle.vert", "assets/shaders/particle.frag");

	// 4. 定义粒子模型 (Quad - 四边形)
		// 格式: [位置 x, y, z] [纹理 u, v]
	float vertices[] = {
		-0.05f, -0.05f, 0.0f,   0.0f, 0.0f,
		 0.05f, -0.05f, 0.0f,   1.0f, 0.0f,
		 0.05f,  0.05f, 0.0f,   1.0f, 1.0f,
		-0.05f,  0.05f, 0.0f,   0.0f, 1.0f
	};
	unsigned int indices[] = {
		0, 1, 3, // 第一个三角形
		1, 2, 3  // 第二个三角形
	};

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// 属性 0: Pos (Stride = 5 * float)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// 属性 1: UV (Offset = 3 * float)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// 5. 加载纹理
	//unsigned int texture1 = loadTexture("assets/textures/texture.png");
	unsigned int texture1 = createSoftTexture();

	// Shader 配置
	ourShader.use();
	ourShader.setInt("particleTexture", 0); // 告诉 Shader 采样器属于 0 号纹理单元

	ParticleSystem* particleSystem = new ParticleSystem(ourShader, 4000); 



	// 渲染循环
	while (!glfwWindowShouldClose(window))
	{
		// 计算 DeltaTime 
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// 输入处理
		processInput(window);

		// ==========================================
		// 1. 逻辑更新 (Update Physics)
		// ==========================================
		// 参数1: dt
		// 参数2: 每帧产生的新粒子数量 (2个)
		// 参数3: 产生位置的偏移量 (这里用 vec2(0,0)，也就是在世界中心发射)
		particleSystem->Update(deltaTime, 80, glm::vec2(0.0f, 0.0f)); // 每次生成 80 个

		// ==========================================
		// 2. 渲染 (Render)
		// ==========================================
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --- A. 绘制场景中的其他物体 (如果有) ---
		// (比如之前的那个大纹理方块，这里可以先注释掉，专心看粒子，或者留着作为背景)

		// 绑定相机的 View/Projection 矩阵给 Shader
		// 因为 ParticleSystem 复用了同一个 Shader，所以这里设置一次即可
		ourShader.use();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		// --- B. 绘制粒子 ---
		// 必须在设置完 projection/view 之后调用
		particleSystem->Draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	delete particleSystem;
	return 0;
}

// 纹理加载函数
unsigned int loadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	// 翻转 Y 轴 (OpenGL 纹理坐标系原点在左下角，而大部分图片数据从左上角开始)
	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D); // 生成多级渐远纹理 (Mipmaps)

		// 纹理参数设置
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data); // 释放 CPU 内存
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// 传递 CameraMovement 枚举给 Camera 类
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
	// 支持空格上升，Shift下降
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraMovement::UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraMovement::DOWN, deltaTime);

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


unsigned int createSoftTexture()
{
	const int width = 64;
	const int height = 64;
	unsigned char data[width * height * 4];

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			// 归一化坐标 (-1 到 1)
			float nx = (float)x / (width - 1) * 2.0f - 1.0f;
			float ny = (float)y / (height - 1) * 2.0f - 1.0f;
			float dist = sqrt(nx * nx + ny * ny);

			// 核心算法：3次幂衰减，让光点中心很亮，边缘很透
			// 这样拉伸后才像水滴，而不是长条方块
			float alpha = std::max(0.0f, 1.0f - dist);
			alpha = pow(alpha, 3.0f);

			int index = (y * width + x) * 4;
			// 雨滴颜色：带一点点冷色调（蓝白）
			data[index + 0] = 220; // R
			data[index + 1] = 220; // G
			data[index + 2] = 255; // B
			data[index + 3] = (unsigned char)(alpha * 255); // A
		}
	}

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// 必须用 GL_CLAMP_TO_EDGE，否则雨滴边缘可能会有奇怪的线条
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return textureID;
}