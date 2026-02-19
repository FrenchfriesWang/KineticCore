#include <iostream>
#include <vector>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "ParticleSystem.h"

// 函数声明
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int generateProceduralTexture();
unsigned int loadTexture(const char* path);
//// STB_IMAGE_IMPLEMENTATION 宏会让库将实现代码编译进这个 cpp 文件
//// 通常在大型项目中，会专门建立一个 src/stb_impl.cpp 来放这个宏，以加快编译速度
//// 这里为了单文件连贯性，暂且放在 main.cpp 顶部
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

// 屏幕设置
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// 相机实例
Camera camera(glm::vec3(0.0f, 1.7f, 3.0f));

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
	auto shader = std::make_unique<Shader>("assets/shaders/particle.vert", "assets/shaders/particle.frag");

	// 使用 std::unique_ptr 管理 ParticleSystem
	// 5000 个粒子作为起步
	auto particleSystem = std::make_unique<ParticleSystem>(*shader, 10000);

	// 生成纹理
	unsigned int textureID = generateProceduralTexture();




	// ---------------------------------------------------------
	// Ground Initialization (局部变量，栈内存管理数据，智能指针管理Shader)
	// ---------------------------------------------------------

	// 1. 地面 Shader (保持 unique_ptr 风格)
	auto groundShader = std::make_unique<Shader>("assets/shaders/ground.vert", "assets/shaders/ground.frag");

	// 2. 地面顶点数据 (移入 main 内部，拒绝全局污染)
	float planeVertices[] = {
		// positions            // normals         // texcoords
		 25.0f, 0.0f,  25.0f,   0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, 0.0f,  25.0f,   0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, 0.0f, -25.0f,   0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 25.0f, 0.0f,  25.0f,   0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, 0.0f, -25.0f,   0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 25.0f, 0.0f, -25.0f,   0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};

	// 3. 配置 OpenGL 对象
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// 4. 加载 PBR 纹理 (直接调用你已有的 loadTexture)
	unsigned int groundDiff = loadTexture("assets/textures/cobblestone_ground_diff.jpg");
	unsigned int groundNorm = loadTexture("assets/textures/cobblestone_ground_nor_gl.jpg");
	unsigned int groundRough = loadTexture("assets/textures/cobblestone_ground_rough.jpg");
	unsigned int groundAO = loadTexture("assets/textures/cobblestone_ground_ao.jpg");
	unsigned int groundDisp = loadTexture("assets/textures/cobblestone_ground_disp.jpg");

	// 5. 预设 Shader 纹理单元
	groundShader->use();
	groundShader->setInt("albedoMap", 0);
	groundShader->setInt("normalMap", 1);
	groundShader->setInt("roughnessMap", 2);
	groundShader->setInt("aoMap", 3);
	groundShader->setInt("dispMap", 4);



	// ------------------------------
	// 5. 渲染循环
	// ------------------------------
	while (!glfwWindowShouldClose(window))
	{
		// 计算 DeltaTime 
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// 输入处理
		processInput(window);

		// 清屏 (背景色设为深色，接近纯黑的虚空感)
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --- 1. 统一计算矩阵 (供所有 Shader 使用) ---
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		// --- 2. 渲染地面 (PBR Wetness) ---
		groundShader->use();
		groundShader->setFloat("time", static_cast<float>(glfwGetTime()));
		groundShader->setMat4("projection", projection);
		groundShader->setMat4("view", view);
		groundShader->setMat4("model", model);

		// 设置光照与湿润参数
		groundShader->setVec3("viewPos", camera.Position);
		groundShader->setVec3("lightPos", glm::vec3(0.0f, 10.0f, 0.0f));
		groundShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		groundShader->setFloat("wetness", 0.45f); // <--- 设为 1.0 满湿润度，强制看效果

		// 绑定纹理
		glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, groundDiff);
		glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, groundNorm);
		glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, groundRough);
		glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, groundAO);
		glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, groundDisp);

		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// --- 3. 渲染粒子 (Transparent Object 放在最后) ---

		// 激活着色器
		shader->use();
		shader->setInt("particleTexture", 0);
		// 设置摄像机矩阵
		shader->setMat4("projection", projection);
		shader->setMat4("view", view);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);


		// 传递 Camera XZ 坐标以实现跟随
		// [重要] 分离更新与渲染
		// ParticleSystem::Update 和 Draw 还是耦合在一起的 (都在 Update 里或者混着)
		// 这一步我们先把调用方式改得像样一点，下一阶段再去拆解 ParticleSystem 类

		// 这里的 offset 暂时没用，先传个 0
		particleSystem->Update(deltaTime, 2, glm::vec2(0.0f, 0.0f));
		particleSystem->Draw(); // 这一步在你的类里虽然包含在Update里了，但为了语义清晰，以后要拆出来


		// 交换缓冲 & 轮询事件
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ------------------------------
	// 6. 资源释放
	// ------------------------------
	// unique_ptr 会在这里自动释放 particleSystem 和 shader，无需 delete
	// 只需处理 OpenGL 的资源
	glDeleteTextures(1, &textureID);
	glfwTerminate();
	return 0;
}

//// 纹理加载函数
//unsigned int loadTexture(const char* path)
//{
//	unsigned int textureID;
//	glGenTextures(1, &textureID);
//
//	int width, height, nrComponents;
//	// 翻转 Y 轴 (OpenGL 纹理坐标系原点在左下角，而大部分图片数据从左上角开始)
//	stbi_set_flip_vertically_on_load(true);
//
//	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
//	if (data)
//	{
//		GLenum format;
//		if (nrComponents == 1)
//			format = GL_RED;
//		else if (nrComponents == 3)
//			format = GL_RGB;
//		else if (nrComponents == 4)
//			format = GL_RGBA;
//
//		glBindTexture(GL_TEXTURE_2D, textureID);
//		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//		glGenerateMipmap(GL_TEXTURE_2D); // 生成多级渐远纹理 (Mipmaps)
//
//		// 纹理参数设置
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//		stbi_image_free(data); // 释放 CPU 内存
//	}
//	else
//	{
//		std::cout << "Texture failed to load at path: " << path << std::endl;
//		stbi_image_free(data);
//	}
//
//	return textureID;
//}

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
	// [补充建议]：在这里加一行锁定高度的代码，虽然我还没写进 Camera 类，但你可以先硬编码
	// camera.Position.y = 1.7f;
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


// 程序化生成雨滴纹理
unsigned int generateProceduralTexture()
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width = 64;
	int height = 64;

	// [修改] 使用 std::vector 自动管理内存，防止 new 后忘记 delete
	std::vector<unsigned char> data(width * height * 4);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// 简单的径向渐变，中心白，边缘透
			float centerX = width / 2.0f;
			float centerY = height / 2.0f;
			float distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
			float maxDist = width / 2.0f;
			float alpha = 1.0f - (distance / maxDist);
			if (alpha < 0.0f) alpha = 0.0f;

			// 稍微让 Alpha 曲线更陡峭一点，粒子边缘更锐利
			alpha = pow(alpha, 3.0f);

			int index = (y * width + x) * 4;
			data[index + 0] = 200; // R (微蓝)
			data[index + 1] = 200; // G
			data[index + 2] = 255; // B
			data[index + 3] = (unsigned char)(alpha * 255); // A
		}
	}

	glBindTexture(GL_TEXTURE_2D, textureID);
	// 使用 data.data() 获取指针
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return textureID;
}


unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1) format = GL_RED;
		else if (nrComponents == 3) format = GL_RGB;
		else if (nrComponents == 4) format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// 设置重复平铺 (GL_REPEAT)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// 地面顶点数据 (Pos, Normal, TexCoords)
// 放在 y = -2.0 的位置，这也是雨滴重置的高度
float planeVertices[] = {
	// positions            // normals         // texcoords
	 25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
	-25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
	-25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

	 25.0f, -2.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
	-25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
	 25.0f, -2.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
};