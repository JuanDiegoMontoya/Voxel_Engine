#include "stdafx.h"
#include <Engine.h>
#include "Interface.h"
#include "World.h"
#include "Renderer.h"
#include "NuRenderer.h"
#include "BitArray.h"
//#include <bitset>
#include "ChunkVBOAllocator.h"

int main()
{
	//BitArray coom(50);
	//coom.SetSequence(5, 8, 0b11001101);
	//std::bitset<8> bb(coom.GetSequence(5, 8));
	//std::cout << bb << std::endl;


	EngineConfig cfg;
	cfg.verticalSync = true;
	Engine::Init(cfg);

	//ChunkVBOAllocator allocator(25000);
	//for (int i = 1; i < 51; i++)
	//	std::cout << std::boolalpha << allocator.Allocate((Chunk*)i, 0, rand() % 500) << std::endl;
	//std::cout << std::boolalpha << allocator.Allocate((Chunk*)69, 0, 25100) << std::endl;
	////std::cout << std::boolalpha << allocator.Free((Chunk*)21) << std::endl;
	//for (int i = 0; i < 50; i++)
	//	std::cout << std::boolalpha << allocator.Free((Chunk*)i) << std::endl;
	//std::cout << std::boolalpha << allocator.FreeOldest() << std::endl;

	Renderer::Init();
	NuRenderer::Init();
	Interface::Init();
	World::Init();

	Engine::Run();

	Engine::Cleanup();

	return 0;
}