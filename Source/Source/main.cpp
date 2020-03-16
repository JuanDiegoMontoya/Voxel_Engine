#include "stdafx.h"

#include <Engine.h>
#include "Interface.h"
#include "World.h"
#include "Renderer.h"

int main()
{
	EngineConfig cfg;
	cfg.verticalSync = true;
	Engine::Init(cfg);
	Renderer::Init();
	Interface::Init();
	World::Init();

	Engine::Run();

	Engine::Cleanup();

	return 0;
}