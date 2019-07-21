#include "stdafx.h"
//#include "biome.h"
#include "block.h"
#include "level.h"
#include "chunk.h"
#include "utilities.h"
#include "generation.h"
#include <noise/noise.h>
#include "vendor/noiseutils.h"
#include <execution>

int maxHeight = 255;

using namespace noise;

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

void WorldGen::GenerateHeightMapWorld(int x, int z, LevelPtr level)
{
	module::DEFAULT_PERLIN_FREQUENCY;
	module::DEFAULT_PERLIN_LACUNARITY;
	module::DEFAULT_PERLIN_OCTAVE_COUNT;
	module::DEFAULT_PERLIN_PERSISTENCE;
	module::DEFAULT_PERLIN_QUALITY;
	module::DEFAULT_PERLIN_SEED;

	module::DEFAULT_RIDGED_FREQUENCY;
	module::DEFAULT_RIDGED_LACUNARITY;
	module::DEFAULT_RIDGED_OCTAVE_COUNT;
	module::DEFAULT_RIDGED_QUALITY;
	module::DEFAULT_RIDGED_SEED;

	module::Perlin surface;
	surface.SetLacunarity(1);
	surface.SetOctaveCount(3);
	surface.SetFrequency(.2);
	surface.SetPersistence(.5);

	module::RidgedMulti tunneler;
	// higher lacunarity = thinner tunnels
	tunneler.SetLacunarity(2.);
	// connectivity/complexity of tunnels (unsure)
	tunneler.SetOctaveCount(5);
	// higher frequency = more common, thicker tunnels 
	// raise lacunarity as frequency decreases
	tunneler.SetFrequency(.01);
	module::Invert inverter;
	inverter.SetSourceModule(0, tunneler);

	//module::Cylinders one;
	//module::RidgedMulti two;
	//module::Perlin control;
	//module::Blend blender;
	//blender.SetSourceModule(0, one);
	//blender.SetSourceModule(1, two);
	//blender.SetControlModule(control);

	//module::Const one1;
	//module::Voronoi two1;
	//module::Perlin control1;
	//module::Blend blender1;
	//blender1.SetSourceModule(0, one1);
	//blender1.SetSourceModule(1, two1);
	//blender1.SetControlModule(blender);

	//module::Blend poop;
	//poop.SetSourceModule(0, blender);
	//poop.SetSourceModule(1, blender1);
	//poop.SetControlModule(blender1);
	//noise.GetValue()
	utils::NoiseMap heightMap;
	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(surface);
	heightMapBuilder.SetDestNoiseMap(heightMap);
	heightMapBuilder.SetDestSize(Chunk::CHUNK_SIZE, Chunk::CHUNK_SIZE);

	int counter = 0;
	for (int xc = 0; xc < x; xc++)
	{
		for (int zc = 0; zc < z; zc++)
		{
			heightMapBuilder.SetBounds(xc, xc + 1, zc, zc + 1);
			heightMapBuilder.Build();

			// generate EVERYTHING
			for (int i = 0; i < Chunk::CHUNK_SIZE; i++)
			{
				for (int k = 0; k < Chunk::CHUNK_SIZE; k++)
				{
					int worldX = xc * Chunk::CHUNK_SIZE + i;
					int worldZ = zc * Chunk::CHUNK_SIZE + k;
					//utils::Color* color = height.GetSlabPtr(i, k);
					float height = *heightMap.GetConstSlabPtr(i, k);

					int y = Utils::mapToRange(height, -1.f, 1.f, -0.f, 100.f);
					level->UpdateBlockAt(glm::ivec3(worldX, y, worldZ), Block::BlockType::bGrass);

					// generate subsurface
					for (int j = -10; j < y; j++)
						level->UpdateBlockAt(glm::ivec3(worldX, j, worldZ), Block::BlockType::bStone);

					// extra layer(s) of grass on the top
					level->UpdateBlockAt(glm::ivec3(worldX, y - 1, worldZ), Block::BlockType::bDirt);
					level->UpdateBlockAt(glm::ivec3(worldX, y - 2, worldZ), Block::BlockType::bDirt);
					level->UpdateBlockAt(glm::ivec3(worldX, y - 3, worldZ), Block::BlockType::bDirt);
				}
			}

			// generate tunnels
			std::for_each(
				std::execution::seq,
				Chunk::chunks.begin(),
				Chunk::chunks.end(),
				[&](const std::pair<glm::ivec3, Chunk*>& p)
			{
				if (!p.second)
					return;
				for (int xb = 0; xb < Chunk::CHUNK_SIZE; xb++)
				{
					for (int yb = 0; yb < Chunk::CHUNK_SIZE; yb++)
					{
						for (int zb = 0; zb < Chunk::CHUNK_SIZE; zb++)
						{
							glm::dvec3 pos = (p.first * Chunk::CHUNK_SIZE) + glm::ivec3(xb, yb, zb);
							double val = tunneler.GetValue(pos.x, pos.y, pos.z);
							//std::cout << val << '\n';
							if (val > .9)
								p.second->At(xb, yb, zb).SetType(Block::bAir);
						}
					}
				}
			});

			//utils::WriterBMP writer;
			//writer.SetSourceImage(image);
			//writer.SetDestFilename(std::string("tutorial" + std::to_string(counter++) + ".bmp"));
			//writer.WriteDestFile();
		}
	}


}

void WorldGen::GenerateChunk(glm::ivec3 cpos, ChunkPtr chunk)
{
	// sample the image that defines which biome each chunk is
}
