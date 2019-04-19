#include "stdafx.h"
#include "sys_window.h"
#include "input.h"

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Sicko Engine", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glViewport(0, 0, 800, 600);
	//glfwSetFramebufferSizeCallback(window, );
	set_glfw_callbacks(window);

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.7f, 0.9f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
		
		
		if (Input::Keyboard().down[GLFW_KEY_ESCAPE])
			glfwSetWindowShouldClose(window, GL_TRUE);
		Input::update();
		//glfwPollEvents();
	}
	glfwTerminate();

	return 0;
}