#include "stdafx.h"

#include "sys_window.h"
#include "input.h"
#include "pipeline.h"
#include "game.h"
#include "imgui_impl.h"

GLFWwindow* win = nullptr;

int main()
{
	GLFWwindow* window = init_glfw_context();
	win = window;

	ImGui_Impl::Init(window);

	glfwMakeContextCurrent(window);
	set_glfw_callbacks(window);

	// 1 = vsync; 0 = fast fps
	glfwSwapInterval(1);

	Render::Init();

	Game game(window);
	game.Run();

	ImGui_Impl::Cleanup();
	glfwTerminate();

	return 0;
}