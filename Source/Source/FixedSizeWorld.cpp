#include "stdafx.h"
#include "FixedSizeWorld.h"
#include "ChunkStorage.h"
#include "generation.h"
#include <execution>


void FixedSizeWorld::GenWorld(glm::uvec3 chunkDim)
{
	for (int x = 0; x < chunkDim.x; x++)
	{
		for (int y = 0; y < chunkDim.y; y++)
		{
			for (int z = 0; z < chunkDim.z; z++)
			{
				Chunk* newChunk = new Chunk();
				ChunkStorage::GetMapRaw()[{ x, y, z }] = newChunk;
				WorldGen::GenerateChunk({ x, y, z });
			}
		}
	}

	auto& chunks = ChunkStorage::GetMapRaw();
	auto lambruh = [&]()
	{
		std::for_each(std::execution::par,
			chunks.begin(), chunks.end(), [](auto& p)
		{
			p.second->BuildMesh();
		});
	};
	lambruh();
	//std::thread coom = std::thread(lambruh);
}
