#include "stdafx.h"
#include "game.h"
#include "level.h"
#include "render.h"
#include "pipeline.h"
#include "input.h"
#include "imgui_impl.h"
#include <algorithm>
#include "chunk.h"

#define IMGUI_ENABLED 1

// initialize
Game::Game(GLFWwindow* window)
{
	window_ = window;
}

// cleans up objects generated during the creation of the game
Game::~Game()
{

}

void Game::Run()
{
	LevelPtr level = new Level("testery");
	level->game_ = this;
	level->Init();

	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	while (!glfwWindowShouldClose(window_))
	{
		float currFrame = (float)glfwGetTime();
		static float oldFrame = 0;
		dt_ = currFrame - oldFrame;
		oldFrame = currFrame;

		const float* clr = &level->GetBgColor()[0];
		glClearColor(clr[0], clr[1], clr[2], 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if IMGUI_ENABLED
		ImGui_Impl::StartFrame();
#endif

		//Render::Draw(level);
		//Render::drawImGui();

		level->Update(dt_);

		if (Input::Keyboard().down[GLFW_KEY_ESCAPE])
			glfwSetWindowShouldClose(window_, GL_TRUE);

		if (Input::Keyboard().down[GLFW_KEY_LEFT_SHIFT])
		{
			if (Input::Keyboard().pressed[GLFW_KEY_1])
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			if (Input::Keyboard().pressed[GLFW_KEY_2])
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			if (Input::Keyboard().pressed[GLFW_KEY_3])
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		}

#if IMGUI_ENABLED
		ImGui_Impl::EndFrame();
#endif
		glfwSwapBuffers(window_);
		Input::update();
	}
}

void Game::Update(float dt)
{

}
