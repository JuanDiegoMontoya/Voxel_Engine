#include "stdafx.h"
#include "block.h"
#include "biome.h"

const Biome& BiomeManager::GetBiome(float temp, float humid, WorldGen::TerrainType terrain)
{
	// track nearest biome to conditions at current block
	std::string currID("error");
	glm::vec2 ogPos(temp, humid);
	float closest = std::numeric_limits<float>::max();
	for (const auto& b : biomes)
	{
		if (b.second.terrain == terrain) // must be matching terrain type
		{
			float dist = glm::distance(ogPos, glm::vec2(b.second.temp_avg, b.second.humidity_avg));
			if (dist <= closest)
			{
				closest = dist;
				currID = b.second.name;
			}
		}
	}
	return biomes[currID];
}

// register hard-coded biomes first, then load custom biomes
void BiomeManager::InitializeBiomes()
{
	// error/fallback biome
	{
		Biome error;
		error.name = "error";
		error.humidity_avg = 9001;
		error.temp_avg = 9001;
		error.terrain = WorldGen::tNone;
		error.surfaceCover = Block::bError;
		registerBiome(error);
	}

	// woodlands
	{
		Biome woodlands(0, 0, WorldGen::tPlains, Block::bGrass);
		woodlands.name = "woodlands";
		woodlands.surfaceFeatures.push_back({ .01f, Prefab::OakTree });
		registerBiome(woodlands);
	}

	// snow hills
	{
		Biome snowH(.5f, -.5f, WorldGen::tHills, Block::bSnow);
		snowH.name = "snow hills";
		snowH.surfaceFeatures.push_back({ .002f, Prefab::BorealTree });
		registerBiome(snowH);
	}

	// flat desert
	{
		Biome desertF(-.5f, .5f, WorldGen::tPlains, Block::bSand);
		desertF.name = "flat desert";
		registerBiome(desertF);
	}
	initCustomBiomes();
}

void BiomeManager::registerBiome(const Biome & biome)
{
	for (const auto& str : biome.biomeOverride)
		biomes.erase(str);
	biomes[biome.name] = biome;
}

Biome BiomeManager::loadBiome(std::string name)
{
	return Biome();
}

void BiomeManager::initCustomBiomes()
{
	return;
	// iterate over files in a special directory and add them as biomes
}
