#include "stdafx.h"
#include "prefab.h"

std::map<Prefab::PrefabName, Prefab> PrefabManager::prefabs_;

void PrefabManager::InitPrefabs()
{
	// add basic tree prefab to list
	Prefab tree;
	for (int i = 0; i < 5; i++)
	{
		tree.Add({ 0, i, 0 }, Block(Block::bOakWood, 2));

		if (i > 2)
		{
			tree.Add({ -1, i, 0 }, Block(Block::bOakLeaves, 1));
			tree.Add({ +1, i, 0 }, Block(Block::bOakLeaves, 1));
			tree.Add({ 0, i, -1 }, Block(Block::bOakLeaves, 1));
			tree.Add({ 0, i, +1 }, Block(Block::bOakLeaves, 1));
		}

		if (i == 4)
			tree.Add({ 0, i + 1, 0 }, Block(Block::bOakLeaves, 1));
	}
	prefabs_[Prefab::OakTree] = tree;

	Prefab bTree;
	for (int i = 0; i < 8; i++)
	{
		if (i < 7)
			bTree.Add({ 0, i, 0 }, Block(Block::bOakWood, 2));
		else
			bTree.Add({ 0, i, 0 }, Block(Block::bOakLeaves, 2));

		if (i > 4)
		{
			bTree.Add({ -1, i, 0 }, Block(Block::bOakLeaves, 1));
			bTree.Add({ +1, i, 0 }, Block(Block::bOakLeaves, 1));
			bTree.Add({ 0 , i, -1 }, Block(Block::bOakLeaves, 1));
			bTree.Add({ 0 , i, +1 }, Block(Block::bOakLeaves, 1));

			bTree.Add({ -1, i, -1 }, Block(Block::bOakLeaves, 1));
			bTree.Add({ +1, i, +1 }, Block(Block::bOakLeaves, 1));
			bTree.Add({ +1, i, -1 }, Block(Block::bOakLeaves, 1));
			bTree.Add({ -1, i, +1 }, Block(Block::bOakLeaves, 1));
		}
	}
	prefabs_[Prefab::OakTreeBig] = bTree;
}
