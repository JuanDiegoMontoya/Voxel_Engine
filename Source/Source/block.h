#pragma once
//#include "level.h"
//#include "pipeline.h"

// visual properties (for now)
struct BlockProperties
{
	BlockProperties(float s, glm::vec4 c)
	: color(c), specular(s), invisible(c.a == 0) {}
	float specular; // shininess
	glm::vec4 color; // diffuse color
	bool invisible; // skip rendering if true
};

// a 1x1x1 cube
typedef class Block
{
public:

	// defines various block properties and behaviors
	enum BlockType : int
	{
		bAir = 0, // default type
		bStone,
		bDirt,
		bMetal,
		bGrass,
		bSand,
		bSnow,

		bCount
	};

	Block(BlockType t = bAir)	: type_(t) {}

	inline BlockType GetType() const { return type_; }
	inline void SetType(BlockType ty) { type_ = ty; }

	static const std::vector<BlockProperties> PropertiesTable;

private:
	BlockType type_ = bAir;
}Block, *BlockPtr;

glm::ivec3 stretch(int index, int w, int h);