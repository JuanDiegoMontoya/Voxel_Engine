#include "stdafx.h"
#include "pipeline.h"
#include "block.h"

const std::vector<BlockProperties> Block::PropertiesTable =
{
	{BlockProperties(0,		glm::vec4(0))},												// air
	{BlockProperties(32,	glm::vec4(.4f, .4f, .4f, 1))},				// stone
	{BlockProperties(16,	glm::vec4(.6f, .3f, .1f, 1))},				// dirt
	{BlockProperties(128,	glm::vec4(.9f, .9f, 1.f, 1))},				// metal
	{BlockProperties(8,		glm::vec4(0, 1, 0, 1))},							// grass
	{BlockProperties(4,		glm::vec4(.761f, .698f, .502f, 1))},	// sand
	{BlockProperties(4,		glm::vec4(1, .98f, .98f, 1))},				// snow
	{BlockProperties(256,	glm::vec4(0, .476f, .745f, .9f))},		// water
	{BlockProperties(1,		glm::vec4(.729f, .643f, .441f, 1))},	// oak wood
	{BlockProperties(1,		glm::vec4(.322f, .42f, .18f, 1))}			// oak leaves
};

glm::ivec3 stretch(int index, int w, int h)
{
	sizeof(Block);
	int z = index / (w * h);
	index -= (z * w * h);
	int y = index / w;
	int x = index % w;
	return glm::vec3(x, y, z);
}