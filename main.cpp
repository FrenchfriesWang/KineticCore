#include <GLFW/glfw3.h>

int main(void){
	GLFWwindow* window;

	if(!glfwInit())
		return -1;
	
	window = glfwCreateWindow(640, 480, "KineticCore - Hello World", NULL, NULL);
	
	if(!window){
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	while(!glfwWindowShouldClose(window)){
		/* 渲染代码 */
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();
	return 0;
}