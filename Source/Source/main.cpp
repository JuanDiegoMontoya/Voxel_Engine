#include "stdafx.h"
#include "sys_window.h"
#include "input.h"
#include "pipeline.h"

int main()
{
	GLFWwindow* window = init_glfw_context();

	glfwMakeContextCurrent(window);
	set_glfw_callbacks(window);

	// basically vsync
	glfwSwapInterval(1);

	Render::Init();
	while (!glfwWindowShouldClose(window))
	{
		Render::Draw();
		
		if (Input::Keyboard().down[GLFW_KEY_ESCAPE])
			glfwSetWindowShouldClose(window, GL_TRUE);

		if (Input::Keyboard().down[GLFW_KEY_1])
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		if (Input::Keyboard().down[GLFW_KEY_2])
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (Input::Keyboard().down[GLFW_KEY_3])
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

		glfwSwapBuffers(window);
		Input::update();
	}
	glfwTerminate();

	return 0;
}