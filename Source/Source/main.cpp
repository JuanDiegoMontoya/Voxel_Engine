#include "stdafx.h"

#include "Renderer.h"
#include <Engine.h>
#include "Interface.h"

int main()
{
	EngineConfig cfg;
	cfg.verticalSync = true;
	Engine::Init(cfg);
	Renderer::Init();
	Interface::Init();

	Engine::Run();

	Engine::Cleanup();

	return 0;
}