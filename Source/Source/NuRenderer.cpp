#include "stdafx.h"
#include "NuRenderer.h"
#include "Renderer.h" // for old rendering functions
#include "World.h"
#include <Engine.h>

namespace NuRenderer
{
	void Init()
	{
		Engine::PushRenderCallback(DrawAll, 0);
	}

	void Clear()
	{
		auto cc = World::bgColor_;
		glClearColor(cc.r, cc.g, cc.b, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void DrawAll()
	{
		PERF_BENCHMARK_START;

		Clear();
		glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction

		Renderer::drawSky();
		drawChunks();
		drawChunksWater();
		Renderer::drawAxisIndicators();
		//Renderer::postProcess();

		glDisable(GL_FRAMEBUFFER_SRGB);
		PERF_BENCHMARK_END;
	}

	void drawChunks()
	{

	}

	void drawChunksWater()
	{

	}
}