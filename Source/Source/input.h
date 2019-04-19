#pragma once
#include "stdafx.h"

namespace Input
{
	struct mouse_input
	{
		bool pressed[GLFW_MOUSE_BUTTON_LAST];
		bool down[GLFW_MOUSE_BUTTON_LAST];
		bool released[GLFW_MOUSE_BUTTON_LAST];

		glm::vec2 screenPos, worldPos;
		glm::vec2 scrollOffset;
	};

	struct kb_input
	{
		bool pressed[GLFW_KEY_LAST];
		bool down[GLFW_KEY_LAST];
		bool released[GLFW_KEY_LAST];
	};

	void init_glfw_input_cbs(GLFWwindow* window);
	void update();
	const kb_input& Keyboard();
	const mouse_input& Mouse();
}
