#pragma once
#include "block.h"

// an object designed to be pasted into the world
struct Prefab
{
	enum PrefabName : int
	{
		OakTree,
		OakTreeBig,

		pfCount
	};

	void Add(glm::ivec3 pos, Block block)
	{
		blocks.push_back(std::pair<glm::ivec3, Block>(pos, block));
	}

	// blocks and their positions relative to the spawn point of the prefab
	std::vector<std::pair<glm::ivec3, Block>> blocks;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(blocks);
	}
};

class PrefabManager
{
public:
	static void InitPrefabs();
	static const Prefab& GetPrefab(Prefab::PrefabName p) { return prefabs_[p]; }

private:
	static void LoadPrefabFromFile(std::string path);

	static std::map<Prefab::PrefabName, Prefab> prefabs_;
};