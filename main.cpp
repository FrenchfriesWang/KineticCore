#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow* window);


int main(void){
	GLFWwindow* window;

	if(!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	window = glfwCreateWindow(640, 480, "KineticCore - Hello World", NULL, NULL);
	
	if(!window){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	while (!glfwWindowShouldClose(window))
	{
		// A. 处理输入
		processInput(window);

		// B. 渲染指令
		// 设置清屏颜色 (R, G, B, Alpha) -> 这里设为深灰色
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		// 执行清屏 (清除颜色缓冲)
		glClear(GL_COLOR_BUFFER_BIT);

		// C. 交换缓冲并检查事件
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

// 函数实现：输入处理
void processInput(GLFWwindow* window)
{
	// 如果按下了 ESC 键，将窗口设置为“应该关闭”
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// 函数实现：视口调整
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// 告诉 OpenGL渲染窗口的尺寸（左下角x, 左下角y, 宽, 高）
	glViewport(0, 0, width, height);
}