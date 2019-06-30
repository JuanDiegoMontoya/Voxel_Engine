#include "stdafx.h"
#include "vbo.h"
#include "vao.h"
#include "chunk.h"
#include "block.h"

std::unordered_map<glm::ivec3, ChunkPtr, Chunk::ivec3Hash> Chunk::activechunks;

Chunk::Chunk()
{
}

Chunk::~Chunk()
{

}

void Chunk::Update()
{
	// update all blocks within the chunk
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				blocks[ID3D(x, y, z, CHUNK_SIZE, CHUNK_SIZE)].Update();
			}
		}
	}
}

void Chunk::Render()
{
}

glm::ivec3 Chunk::worldToChunkPos()
{
	return glm::ivec3();
}

void Chunk::buildMesh()
{
}
