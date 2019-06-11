#include "stdafx.h"
#include "imgui_impl.h"

namespace ImGui_Impl
{
	void Init(GLFWwindow* window)
	{
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init();
		ImGui::StyleColorsDark();
	}

	void StartFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void EndFrame()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::EndFrame();
	}

	void Cleanup()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui::DestroyContext();
	}
}