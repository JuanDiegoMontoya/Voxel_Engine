#include "stdafx.h"
#include "game.h"
#include "level.h"
#include "render.h"
#include "pipeline.h"
#include "input.h"
#include "imgui_impl.h"

// initialize
Game::Game(GLFWwindow* window)
{
	_window = window;
}

// cleans up objects generated during the creation of the game
Game::~Game()
{

}

void Game::Run()
{
	LevelPtr level = new Level("testery");
	level->Init();

	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	while (!glfwWindowShouldClose(_window))
	{
		float currFrame = (float)glfwGetTime();
		static float oldFrame = 0;
		_dt = currFrame - oldFrame;
		oldFrame = currFrame;

		glClearColor(0, 0, 0, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_Impl::StartFrame();

		Render::Draw(level);
		//Render::drawImGui();

		level->Update(_dt);

		if (Input::Keyboard().down[GLFW_KEY_ESCAPE])
			glfwSetWindowShouldClose(_window, GL_TRUE);

		if (Input::Keyboard().down[GLFW_KEY_1])
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		if (Input::Keyboard().down[GLFW_KEY_2])
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (Input::Keyboard().down[GLFW_KEY_3])
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

		ImGui_Impl::EndFrame();
		glfwSwapBuffers(_window);
		Input::update();
	}
}

void Game::Update(float dt)
{

}
