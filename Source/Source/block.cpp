#include "stdafx.h"
#include "pipeline.h"
#include "block.h"

extern std::vector<unsigned> Render::updatedBlocks;
std::vector<unsigned>* Block::updateList_ = &Render::updatedBlocks;
unsigned Block::count_ = 0;
Block Block::blocksarr_[100 * 100 * 100]; // one million positions

void Block::Update(float dt)
{
	// check if model needs to be computed
}

glm::ivec3 stretch(int index, int w, int h)
{
	int z = index / (w * h);
	index -= (z * w * h);
	int y = index / w;
	int x = index % w;
	return glm::vec3(x, y, z);
}