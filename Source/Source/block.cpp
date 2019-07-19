#include "stdafx.h"
#include "pipeline.h"
#include "block.h"

const std::vector<BlockProperties> Block::PropertiesTable =
{
	{BlockProperties(0,		glm::vec4(0))},
	{BlockProperties(32,	glm::vec4(.4f, .4f, .4f, 1))},
	{BlockProperties(16,	glm::vec4(.6f, .3f, .1f, 1))},
	{BlockProperties(128,	glm::vec4(.9f, .9f, 1.f, 1))},
	{BlockProperties(8,		glm::vec4(0, 1, 0, 1))}
};

glm::ivec3 stretch(int index, int w, int h)
{
	int z = index / (w * h);
	index -= (z * w * h);
	int y = index / w;
	int x = index % w;
	return glm::vec3(x, y, z);
}