#pragma once
#include "prefab.h"
#include "biome.h"

typedef struct Chunk* ChunkPtr;
typedef class Level* LevelPtr;

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
	
	static void GenerateHeightMapWorld(int x, int z, LevelPtr level);

	//static void GenerateChunkMap()

	/*
		Populates a chunk based on its position in the world
	*/
	static void GenerateChunk(glm::ivec3 cpos, LevelPtr level);

	static void GeneratePrefab(const Prefab& pfab, glm::ivec3 wpos, LevelPtr level);

	static void Generate3DNoiseChunk(glm::ivec3 cpos, LevelPtr level);

	// returns a density at a particular point
	static double GetCurrentNoise(const glm::vec3& wpos);
private:
	// sample near values in heightmap to obtain rough first derivative
	static float getSlope(noise::model::Plane& pl, int x, int z);

	// you can't make this object
	WorldGen() = delete;


};