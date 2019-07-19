#pragma once

typedef struct Chunk* ChunkPtr;

class WorldGen
{
public:
	/* 
		Generates a rectangular world of the given dimensions
		(in chunks). Sparsity of the blocks can be controlled.
		
		*Size: number of chunks to generate in that direction
		sparse (0-1): chance to generate a non-air block
		updateList: located within level calling this function
	*/
	static void GenerateSimpleWorld(int xSize, int ySize, int zSize, float sparse, std::vector<ChunkPtr>& updateList);

private:
	WorldGen() = delete;
};