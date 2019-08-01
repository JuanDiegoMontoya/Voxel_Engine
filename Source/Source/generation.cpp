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

	module::Const canvas0;
	canvas0.SetConstValue(-1.0);
	module::Const gray;
	gray.SetConstValue(0);

	// add a little bit of plains to a blank canvas
	module::Perlin plains;
	plains.SetLacunarity(1);
	plains.SetOctaveCount(3);
	plains.SetFrequency(.1);
	plains.SetPersistence(.5);
	module::Perlin plainsPicker;
	plainsPicker.SetSeed(0);
	plainsPicker.SetLacunarity(0);
	plainsPicker.SetFrequency(.2);
	plainsPicker.SetOctaveCount(3);
	module::Select plainsSelect;
	plainsSelect.SetBounds(-1, 0);
	plainsSelect.SetEdgeFalloff(.1);
	plainsSelect.SetSourceModule(0, canvas0);
	plainsSelect.SetSourceModule(1, plains);
	plainsSelect.SetControlModule(plainsPicker);
	module::Select canvas1;
	canvas1.SetEdgeFalloff(1.5);
	canvas1.SetSourceModule(0, canvas0);
	canvas1.SetSourceModule(1, plainsSelect);
	canvas1.SetControlModule(canvas0);

	module::Perlin hillsLumpy;
	hillsLumpy.SetFrequency(.2);
	hillsLumpy.SetOctaveCount(2);
	hillsLumpy.SetPersistence(1.5);
	module::Const hillsHeight;
	hillsHeight.SetConstValue(1);
	module::Add hills;
	hills.SetSourceModule(0, hillsLumpy);
	hills.SetSourceModule(1, hillsHeight);
	module::Perlin hillsPicker;
	hillsPicker.SetSeed(1);
	hillsPicker.SetLacunarity(0);
	hillsPicker.SetFrequency(.2);
	hillsPicker.SetOctaveCount(3);
	module::Select hillsSelect;
	hillsSelect.SetBounds(-1, 0);
	hillsSelect.SetEdgeFalloff(.1);
	hillsSelect.SetSourceModule(0, canvas0);
	hillsSelect.SetSourceModule(1, hills);
	hillsSelect.SetControlModule(hillsPicker);
	module::Select canvas2;
	canvas2.SetEdgeFalloff(1.1);
	canvas2.SetBounds(-1, -.5);
	canvas2.SetSourceModule(0, canvas1);
	canvas2.SetSourceModule(1, hillsSelect);
	canvas2.SetControlModule(canvas1);

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
	heightMapBuilder.SetSourceModule(canvas2);
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
					height = height < -1 ? -1 : height;

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
			//std::for_each(
			//	std::execution::par,
			//	Chunk::chunks.begin(),
			//	Chunk::chunks.end(),
			//	[&](const std::pair<glm::ivec3, Chunk*>& p)
			//{
			//	if (!p.second)
			//		return;
			//	for (int xb = 0; xb < Chunk::CHUNK_SIZE; xb++)
			//	{
			//		for (int yb = 0; yb < Chunk::CHUNK_SIZE; yb++)
			//		{
			//			for (int zb = 0; zb < Chunk::CHUNK_SIZE; zb++)
			//			{
			//				glm::dvec3 pos = (p.first * Chunk::CHUNK_SIZE) + glm::ivec3(xb, yb, zb);
			//				double val = tunneler.GetValue(pos.x, pos.y, pos.z);
			//				//std::cout << val << '\n';
			//				if (val > .9)
			//					p.second->At(xb, yb, zb).SetType(Block::bAir);
			//			}
			//		}
			//	}
			//});

			//utils::WriterBMP writer;
			//writer.SetSourceImage(image);
			//writer.SetDestFilename(std::string("tutorial" + std::to_string(counter++) + ".bmp"));
			//writer.WriteDestFile();
		}
	}


}

void WorldGen::GenerateChunk(glm::ivec3 cpos, LevelPtr level)
{
	static bool init = true;
	static module::Const canvas0;
	static module::Const gray;
	static module::Perlin plains;
	static module::Perlin plainsPicker;
	static module::Select plainsSelect;
	static module::Select canvas1;
	static module::Perlin hillsLumpy;
	static module::Const hillsHeight;
	static module::Add hills;
	static module::Perlin hillsPicker;
	static module::Select hillsSelect;
	static module::Select canvas2;
	static module::RidgedMulti tunneler;
	static module::Invert inverter;
	static utils::NoiseMap heightMap;
	static utils::NoiseMapBuilderPlane heightMapBuilder;

	static module::RidgedMulti riversBase;
	static module::Perlin riversRandomizer;
	static module::Turbulence rivers;
	static utils::NoiseMap riverMap;
	static utils::NoiseMapBuilderPlane riverMapBuilder;

	if (init)
	{
		canvas0.SetConstValue(-1.0);
		gray.SetConstValue(0);

		riversBase.SetLacunarity(3.);
		riversBase.SetOctaveCount(1);
		riversBase.SetFrequency(.10);
		riversRandomizer.SetOctaveCount(1);

		rivers.SetSourceModule(0, riversBase);
		rivers.SetRoughness(1);
		
		//rivers.SetPersistence(.1);

		// add a little bit of plains to a blank canvas
		plains.SetLacunarity(0);
		plains.SetOctaveCount(3);
		plains.SetFrequency(.03);
		plains.SetPersistence(.8);
		plainsPicker.SetSeed(0);
		plainsPicker.SetLacunarity(0);
		plainsPicker.SetFrequency(.1);
		plainsPicker.SetOctaveCount(3);
		plainsSelect.SetBounds(-1, 0);
		plainsSelect.SetEdgeFalloff(.1);
		plainsSelect.SetSourceModule(0, canvas0);
		plainsSelect.SetSourceModule(1, plains);
		plainsSelect.SetControlModule(plainsPicker);
		canvas1.SetEdgeFalloff(1.5);
		canvas1.SetSourceModule(0, canvas0);
		canvas1.SetSourceModule(1, plainsSelect);
		canvas1.SetControlModule(canvas0);

		hillsLumpy.SetFrequency(.2);
		hillsLumpy.SetOctaveCount(2);
		hillsLumpy.SetPersistence(1.0);
		hillsHeight.SetConstValue(0);
		hills.SetSourceModule(0, hillsLumpy);
		hills.SetSourceModule(1, hillsHeight);
		hillsPicker.SetSeed(1);
		hillsPicker.SetLacunarity(0);
		hillsPicker.SetFrequency(.1);
		hillsPicker.SetOctaveCount(3);
		hillsSelect.SetBounds(-1, 0);
		hillsSelect.SetEdgeFalloff(.1);
		hillsSelect.SetSourceModule(0, canvas0);
		hillsSelect.SetSourceModule(1, hills);
		hillsSelect.SetControlModule(hillsPicker);
		canvas2.SetEdgeFalloff(1.1);
		canvas2.SetBounds(-1, -.5);
		canvas2.SetSourceModule(0, canvas1);
		canvas2.SetSourceModule(1, hillsSelect);
		canvas2.SetControlModule(canvas1);

		// higher lacunarity = thinner tunnels
		tunneler.SetLacunarity(2.);
		// connectivity/complexity of tunnels (unsure)
		tunneler.SetOctaveCount(5);
		// higher frequency = more common, thicker tunnels 
		// raise lacunarity as frequency decreases
		tunneler.SetFrequency(.01);
		inverter.SetSourceModule(0, tunneler);

		riverMapBuilder.SetSourceModule(rivers);
		riverMapBuilder.SetDestNoiseMap(riverMap);
		riverMapBuilder.SetDestSize(Chunk::CHUNK_SIZE, Chunk::CHUNK_SIZE);

		heightMapBuilder.SetSourceModule(canvas2);
		heightMapBuilder.SetDestNoiseMap(heightMap);
		heightMapBuilder.SetDestSize(Chunk::CHUNK_SIZE, Chunk::CHUNK_SIZE);

		init = false;
	}

	heightMapBuilder.SetBounds(cpos.x, cpos.x + 1, cpos.z , cpos.z + 1);
	heightMapBuilder.Build();
	riverMapBuilder.SetBounds(cpos.x, cpos.x + 1, cpos.z, cpos.z + 1);
	riverMapBuilder.Build();

	// generate EVERYTHING
	for (int i = 0; i < Chunk::CHUNK_SIZE; i++)
	{
		for (int j = 0; j < Chunk::CHUNK_SIZE; j++)
		{
			for (int k = 0; k < Chunk::CHUNK_SIZE; k++)
			{
				int worldX = cpos.x * Chunk::CHUNK_SIZE + i;
				int worldY = cpos.y * Chunk::CHUNK_SIZE + j;
				int worldZ = cpos.z * Chunk::CHUNK_SIZE + k;
				//utils::Color* color = height.GetSlabPtr(i, k);
				float height = *heightMap.GetConstSlabPtr(i, k);
				float riverVal = *riverMap.GetConstSlabPtr(i, k);
				height = height < -1 ? -1 : height;

				int y = Utils::mapToRange(height, -1.f, 1.f, -0.f, 150.f);
				int riverModifier = 0;
				if (riverVal > 0 && getSlope(heightMap, i, k) < 0.004f)
					riverModifier = glm::clamp(Utils::mapToRange(riverVal, 0.05f, 0.2f, 0.f, 5.f), 0.f, 30.f);
				int actualHeight = y - riverModifier;

				//if (worldY > actualHeight && worldY < y)
				//	level->UpdateBlockAt(glm::ivec3(worldX, worldY, worldZ), Block::BlockType::bSand);
				if (worldY > actualHeight && worldY < y - 1)
					level->UpdateBlockAt(glm::ivec3(worldX, worldY, worldZ), Block::BlockType::bWater);

				double val = tunneler.GetValue(worldX, worldY, worldZ); // maybe also use for rivers

				// top cover
				if (worldY == actualHeight)
				{
					if (hillsPicker.GetValue(worldX, worldY, worldZ) * 30 + worldY > 90)
						level->UpdateBlockAt(glm::ivec3(worldX, worldY, worldZ), Block::BlockType::bSnow);
					else
						level->UpdateBlockAt(glm::ivec3(worldX, worldY, worldZ), Block::BlockType::bGrass);
				}
				if (worldY >= actualHeight - 3 && worldY < actualHeight)
					level->UpdateBlockAt(glm::ivec3(worldX, worldY, worldZ), Block::BlockType::bDirt);

				// generate subsurface
				if (worldY >= -10 && worldY < actualHeight - 3)
					level->UpdateBlockAt(glm::ivec3(worldX, worldY, worldZ), Block::BlockType::bStone);
				//if (val > .9)
				//	level->UpdateBlockAt(glm::ivec3(worldX, worldY, worldZ), Block::bAir);
				//if (val > .87 && worldY >= actualHeight - 3 && worldY <= actualHeight + 2)
				//	level->UpdateBlockAt(glm::ivec3(worldX, worldY, worldZ), Block::BlockType::bAir);
			}
		}
	}

	//// generate tunnels
	//for (int xb = 0; xb < Chunk::CHUNK_SIZE; xb++)
	//{
	//	for (int yb = 0; yb < Chunk::CHUNK_SIZE; yb++)
	//	{
	//		for (int zb = 0; zb < Chunk::CHUNK_SIZE; zb++)
	//		{
	//			glm::dvec3 pos = (cpos * Chunk::CHUNK_SIZE) + glm::ivec3(xb, yb, zb);
	//			double val = tunneler.GetValue(pos.x, pos.y, pos.z);
	//			//std::cout << val << '\n';
	//			if (val > .9)
	//				level->UpdateBlockAt(glm::ivec3(xb, yb, zb), Block::bAir);
	//		}
	//	}
	//}
}

void WorldGen::Generate3DNoiseChunk(glm::ivec3 cpos, LevelPtr level)
{
	for (int xb = 0; xb < Chunk::CHUNK_SIZE; xb++)
	{
		for (int yb = 0; yb < Chunk::CHUNK_SIZE; yb++)
		{
			for (int zb = 0; zb < Chunk::CHUNK_SIZE; zb++)
			{
				glm::dvec3 pos = (cpos * Chunk::CHUNK_SIZE) + glm::ivec3(xb, yb, zb);
				double dens = GetCurrentNoise(pos);
				if (dens + .0 < Chunk::isolevel)
					level->UpdateBlockAt(glm::ivec3(pos), Block::bAir);
				else
					level->UpdateBlockAt(glm::ivec3(pos), Block::bGrass);
			}
		}
	}
}

double WorldGen::GetCurrentNoise(const glm::vec3& wpos)
{
	static bool init = true;
	static module::RidgedMulti dense;

	if (init)
	{
		// higher lacunarity = thinner tunnels
		dense.SetLacunarity(2.);
		// connectivity/complexity of tunnels (unsure)
		dense.SetOctaveCount(5);
		// higher frequency = more common, thicker tunnels 
		// raise lacunarity as frequency decreases
		dense.SetFrequency(.01);
		// init generator here
	}

	//return 1;
	return dense.GetValue(wpos.x, wpos.y, wpos.z);
}

// TODO: make this function able to look past the end of the heightmap
float WorldGen::getSlope(utils::NoiseMap& heightmap, int x, int z)
{
	//float height = *heightmap.GetConstSlabPtr(x, z);
	float height = heightmap.GetValue(x, z);

	// Compute the differentials by stepping over 1 in both directions.
	float val1 = heightmap.GetValue(x + 1, z);
	float val2 = heightmap.GetValue(x, z + 1);
	//if (val1 == 0 || val2 == 0)
	//	return 0;
	float dx = val1 - height;
	float dz = val2 - height;
	
	// The "steepness" is the magnitude of the gradient vector
	// For a faster but not as accurate computation, you can just use abs(dx) + abs(dy)
	return glm::sqrt(dx * dx + dz * dz);
}
