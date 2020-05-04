#include "stdafx.h"
#include <Engine.h>
#include "Interface.h"
#include "World.h"
#include "Renderer.h"
#include "NuRenderer.h"
#include "BitArray.h"
//#include <bitset>

int main()
{
	//BitArray coom(50);
	//coom.SetSequence(5, 8, 0b11001101);
	//std::bitset<8> bb(coom.GetSequence(5, 8));
	//std::cout << bb << std::endl;

	EngineConfig cfg;
	cfg.verticalSync = true;
	Engine::Init(cfg);
	Renderer::Init();
	NuRenderer::Init();
	Interface::Init();
	World::Init();

	Engine::Run();

	Engine::Cleanup();

	return 0;
}