#pragma once

namespace ImGui_Impl
{
	void Init(GLFWwindow* window);
	void StartFrame();
	void EndFrame();
	void Cleanup();
}