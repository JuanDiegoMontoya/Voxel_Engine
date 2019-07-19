#include "stdafx.h"
#include "level.h"
#include "chunk.h"
#include "utilities.h"
#include "generation.h"

void WorldGen::GenerateSimpleWorld(int xSize, int ySize, int zSize, float sparse, std::vector<ChunkPtr>& updateList)
{	// this loop is not parallelizable (unless VAO constructor is moved)
	for (int xc = 0; xc < xSize; xc++)
	{
		for (int yc = 0; yc < ySize; yc++)
		{
			for (int zc = 0; zc < zSize; zc++)
			{
				Chunk* init = Chunk::chunks[glm::ivec3(xc, yc, zc)] = new Chunk(true);
				init->SetPos(glm::ivec3(xc, yc, zc));
				updateList.push_back(init);
					
				for (int x = 0; x < Chunk::CHUNK_SIZE; x++)
				{
					for (int y = 0; y < Chunk::CHUNK_SIZE; y++)
					{
						for (int z = 0; z < Chunk::CHUNK_SIZE; z++)
						{
							if (Utils::get_random(0, 1) > sparse)
								continue;
							init->At(x, y, z).SetType(static_cast<Block::BlockType>(static_cast<int>(Utils::get_random(1, Block::bCount))));
						}
					}
				}
			}
		}
	}
}