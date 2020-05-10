#include "stdafx.h"
#include "WorldGen2.h"
#include "chunk.h"
#include "ChunkStorage.h"
#include "ChunkHelpers.h"
#include <execution>
#include <noise/noise.h>
#include "vendor/noiseutils.h"

namespace WorldGen2
{
	namespace
	{
		glm::ivec3 lowChunkDim{ -3, -3, -3 };
		glm::ivec3 highChunkDim{ 4, 10, 4 };
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


	// does the thing
	void GenerateWorld()
	{
		module::Perlin noise;
		module::Checkerboard checky;
		noise.SetLacunarity(2.);
		noise.SetOctaveCount(5);
		noise.SetFrequency(.04);
		
		auto& chunks = ChunkStorage::GetMapRaw();
		std::for_each(std::execution::par, chunks.begin(), chunks.end(),
			[&](auto pair) 
		{
			if (pair.second)
			{
				printf("(%d, %d, %d)\n", pair.first.x, pair.first.y, pair.first.z);
				glm::ivec3 pos, wpos;
				for (pos.z = 0; pos.z < Chunk::CHUNK_SIZE; pos.z++)
				{
					int zcsq = pos.z * Chunk::CHUNK_SIZE_SQRED;
					for (pos.y = 0; pos.y < Chunk::CHUNK_SIZE; pos.y++)
					{
						int yczcsq = pos.y * Chunk::CHUNK_SIZE + zcsq;
						for (pos.x = 0; pos.x < Chunk::CHUNK_SIZE; pos.x++)
						{
							int index = pos.x + yczcsq;
							wpos = ChunkHelpers::chunkPosToWorldPos(pos, pair.first);

							double density = noise.GetValue(pos.x, pos.y, pos.z);

							if (density > .9)
							{
								ChunkStorage::SetBlockType(wpos, BlockType::bStone);
							}
							if (density > .95)
							{
								ChunkStorage::SetBlockType(wpos, BlockType::bDirt);
							}
							if (density <= .9)
							{
								ChunkStorage::SetBlockType(wpos, BlockType::bAir);
							}
							//if (wpos.y < -10)
							//{
							//	ChunkStorage::SetBlockType(wpos, BlockType::bStone);
							//}
							//else if (wpos.y <= 0)
							//{
							//	ChunkStorage::SetBlockType(wpos, BlockType::bDirt);
							//}
							//else if (wpos.y == 1)
							//{
							//	ChunkStorage::SetBlockType(wpos, BlockType::bGrass);
							//}
						}
					}
				}
			}
			else
			{
				printf("null chunk doe\n");
			}
		});
	}


	void InitMeshes()
	{
		auto& chunks = ChunkStorage::GetMapRaw();
		std::for_each(std::execution::par,
			chunks.begin(), chunks.end(), [](auto& p)
		{
			p.second->BuildMesh();
		});
	}


	void InitBuffers()
	{
		auto& chunks = ChunkStorage::GetMapRaw();
		std::for_each(std::execution::seq,
			chunks.begin(), chunks.end(), [](auto& p)
		{
			p.second->BuildBuffers();
		});
	}
}