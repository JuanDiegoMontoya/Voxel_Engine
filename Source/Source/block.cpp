#include "stdafx.h"
#include "pipeline.h"
#include "block.h"

extern std::vector<unsigned> Render::updatedBlocks;
std::vector<unsigned>* Block::_updated = &Render::updatedBlocks;
unsigned Block::_count = 0;

// maybe in the future have this function omit vertices attached to culled sides
std::vector<GLfloat> Block::GetVertices()
{
	vertices.clear();
	for (size_t i = 0; i < _countof(Render::cube_tex_vertices); i++)
	{
		vertices.push_back(Render::cube_tex_vertices[i]);
	}
	return vertices;
}

// called when a chunk updates
// chunks update when any block in them is changed
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