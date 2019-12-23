#pragma once
#include "generation.h"
#include "misc_utils.h"

// TODO: add more properties like grass/water color modifier, etc
//			 sky color,
struct Biome
{
	Biome(float humid, float temp,
		WorldGen::TerrainType tt, Block::BlockType bt)
		: humidity_avg(humid), temp_avg(temp), terrain(tt), surfaceCover(bt) {}
	Biome() {}

	std::string name; // identifier

	// when to spawn this biome
	float humidity_avg; // -1 to 1
	float temp_avg;			// -1 to 1
	WorldGen::TerrainType terrain;

	// % chance and name of prefab to spawn
	Block::BlockType surfaceCover; // sand, dirt, snow, etc.
	std::vector<std::pair<float, PrefabName>> surfaceFeatures;	// per block
	std::vector<std::pair<float, PrefabName>> subFeatures;			// per chunk
	std::vector<std::pair<float, PrefabName>> skyFeatures;			// per chunk

	// deletes these biomes when added (for custom biomes)
	std::vector<std::string> biomeOverride;

	// serialization
	template <class Archive>
	void serialize(Archive& ar)
	{
		// ar()
	}
};

// registers biomes and stuff
class BiomeManager
{
public:
	//struct hashfunc
	//{
	//	size_t operator()(const Biome& b) const
	//	{
	//		return Utils::djb2hash()(b.name.c_str());
	//	}
	//};

	//struct keyeq
	//{
	//	bool operator()(const Biome& first, const Biome& second) const
	//	{
	//		return Utils::charPtrKeyEq()(first.name.c_str(), second.name.c_str());
	//	}
	//};

	inline static std::unordered_map<std::string, Biome> biomes;

	const static Biome& GetBiome(float temp, float humid, WorldGen::TerrainType terrain);
	static void InitializeBiomes();
private:
	static void registerBiome(const Biome& biome);

	static Biome loadBiome(std::string name);
	static void initCustomBiomes();
};