#pragma once
#include "block.h"

// cubic dimensions of a single chunk
//#define CHUNK_SIZE 16

//typedef class Block;

typedef struct Chunk
{
public:
	Chunk();
	~Chunk();

	void Update(float dt);
	void Render();

	static constexpr int CHUNK_SIZE = 16;

	Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

private:
	VAO* vao_;
	VBO* vbo_;
}Chunk, *ChunkPtr;