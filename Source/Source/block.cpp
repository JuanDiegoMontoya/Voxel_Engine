#include "stdafx.h"
#include "pipeline.h"
#include "block.h"

glm::ivec3 stretch(int index, int w, int h)
{
	int z = index / (w * h);
	index -= (z * w * h);
	int y = index / w;
	int x = index % w;
	return glm::vec3(x, y, z);
}