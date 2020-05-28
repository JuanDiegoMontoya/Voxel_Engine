#include "stdafx.h"
#include "WorldGen2.h"
#include "chunk.h"
#include "ChunkStorage.h"
#include "ChunkHelpers.h"
#include <execution>
#include <noise/noise.h>
#include "vendor/noiseutils.h"
#include "vendor/FastNoiseSIMD/FastNoiseSIMD.h"

namespace WorldGen2
{
	namespace
	{
#if 1
		glm::ivec3 lowChunkDim{ 0, 0, 0 };
		glm::ivec3 highChunkDim{ 8, 10, 8 };
#else
		glm::ivec3 lowChunkDim{ 0, 0, 0 };
		glm::ivec3 highChunkDim{ 2, 1, 1 };
#endif
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

	// Parameters: (density > .05) == solid
	//   lacunarity = 2, octaves = 5
	// Value noise: good for structured, geometric looking locations
	//   generates consistently sized caverns with good connectivity
	// Fractal perlin: grand, lumpy, almost chiseled-like terrain
	//   big rocky areas, good connectivity

	// does the thing
	void GenerateWorld()
	{
		module::Perlin noise;
		module::Checkerboard checky;
		noise.SetLacunarity(2.);
		noise.SetOctaveCount(2);
		noise.SetFrequency(.04);
		FastNoiseSIMD* noisey = FastNoiseSIMD::NewFastNoiseSIMD();
		noisey->SetFractalLacunarity(2.0);
		noisey->SetFractalOctaves(5);
		//noisey->SetFrequency(.04);

		
		auto& chunks = ChunkStorage::GetMapRaw();
		std::for_each(std::execution::par, chunks.begin(), chunks.end(),
			[&](auto pair)
		{
			if (pair.second)
			{
				glm::ivec3 st = pair.first * Chunk::CHUNK_SIZE;
				float* noiseSet = noisey->GetCubicFractalSet(st.z, st.y, st.x, 
					Chunk::CHUNK_SIZE, Chunk::CHUNK_SIZE, Chunk::CHUNK_SIZE, 1);
				int idx = 0;

				printf(".");
				glm::ivec3 pos, wpos;
				for (pos.z = 0; pos.z < Chunk::CHUNK_SIZE; pos.z++)
				{
					//int zcsq = pos.z * Chunk::CHUNK_SIZE_SQRED;
					for (pos.y = 0; pos.y < Chunk::CHUNK_SIZE; pos.y++)
					{
						//int yczcsq = pos.y * Chunk::CHUNK_SIZE + zcsq;
						for (pos.x = 0; pos.x < Chunk::CHUNK_SIZE; pos.x++)
						{
							//int index = pos.x + yczcsq;
							wpos = ChunkHelpers::chunkPosToWorldPos(pos, pair.first);

							//double density = noise.GetValue(wpos.x, wpos.y, wpos.z); // chunks are different
							//double density = noise.GetValue(pos.x, pos.y, pos.z); // same chunk every time
							//density = 0;
							float density = noiseSet[idx++];
							if (density > -.03)
							{
								ChunkStorage::SetBlockType(wpos, BlockType::bStone);
							}
							if (density < -.03)
							{
								ChunkStorage::SetBlockType(wpos, BlockType::bDirt);
							}
							if (density >= -.05)
							{
								ChunkStorage::SetBlockType(wpos, BlockType::bAir);
							}
						}
					}
				}

				FastNoiseSIMD::FreeNoiseSet(noiseSet);
			}
			else
			{
				printf("null chunk doe\n");
			}
		});

		delete noisey;
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