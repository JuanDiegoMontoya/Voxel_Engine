#pragma once

// cubic dimensions of a single chunk
//#define CHUNK_SIZE 16

typedef class Block;
typedef class VBO;
typedef class VAO;

typedef struct Chunk
{
public:
	Chunk();
	~Chunk();

	void Update(float dt);
	void Render();

	static constexpr int CHUNK_SIZE = 32;

	// frequently accessed data
	Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

private:
	void GenerateMesh();

	VAO* _vao;
	VBO* _vbo;
}Chunk, *ChunkPtr;