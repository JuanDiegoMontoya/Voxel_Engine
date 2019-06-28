#pragma once

// cubic dimensions of a single chunk
//#define CHUNK_SIZE 16

typedef class Block* BlockPtr;

typedef struct Chunk
{
public:
	Chunk();
	~Chunk();

	void Update(float dt);
	void Render();

	static constexpr int CHUNK_SIZE = 16;

	BlockPtr blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

private:
	VAO* _vao;
	VBO* _vbo;
}Chunk, *ChunkPtr;