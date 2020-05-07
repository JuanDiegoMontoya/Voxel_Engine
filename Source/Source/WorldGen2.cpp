#include "stdafx.h"
#include "WorldGen2.h"
#include "chunk.h"
#include "ChunkStorage.h"
#include "ChunkHelpers.h"

namespace WorldGen2
{
	namespace
	{
		glm::ivec3 lowChunkDim{ -3, -3, -3 };
		glm::ivec3 highChunkDim{ 3, 3, 3 };
	}

	// init chunks that we finna modify
	void Init()
	{
		for (int x = lowChunkDim.x; x < highChunkDim.x; x++)
		{
			printf("\nX: %d", x);
			for (int y = lowChunkDim.y; y < highChunkDim.y; y++)
			{
				printf(" Y: %d", y);
				for (int z = lowChunkDim.z; z < highChunkDim.z; z++)
				{
					Chunk* newChunk = new Chunk();
					newChunk->SetPos({ x, y, z });
					ChunkStorage::GetMapRaw()[{ x, y, z }] = newChunk;
				}
			}
		}
	}

	void GenerateWorld()
	{

	}
}