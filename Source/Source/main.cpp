#include "stdafx.h"
#include "sys_window.h"
#include "input.h"

int main()
{
	GLFWwindow* window = init_glfw_context();

	glfwMakeContextCurrent(window);
	//glViewport(0, 0, 800, 600);
	//glfwSetFramebufferSizeCallback(window, );
	set_glfw_callbacks(window);

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.7f, 0.3f, 0.3f, 1.0f);
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