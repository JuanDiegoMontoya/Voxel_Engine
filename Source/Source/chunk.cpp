#include "stdafx.h"
#include "vbo.h"
#include "vao.h"
#include "chunk.h"
#include "block.h"

Chunk::Chunk()
{
}

Chunk::~Chunk()
{
	// if the blocks are constructed in-place, then we don't need to free them in this destructor
	delete _vao;
	delete _vbo;
	//for (int x = 0; x < CHUNK_SIZE; x++)
	//{
	//	for (int y = 0; y < CHUNK_SIZE; y++)
	//	{
	//		for (int z = 0; z < CHUNK_SIZE; z++)
	//		{
	//			delete blocks[ID3D]
	//		}
	//	}
	//}
}

void Chunk::Update(float dt)
{
}

void Chunk::Render()
{
}

void Chunk::GenerateMesh()
{
	delete _vbo;
	std::vector<float> vertices;
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				if (blocks[ID3D(x, y, z, CHUNK_SIZE, CHUNK_SIZE)].IsEnabled())
				{
					// add the vertices of the block to the vbo

				}
			}
		}
	}
}
