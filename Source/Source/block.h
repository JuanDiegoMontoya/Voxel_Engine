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
#pragma pack(push, 1)
typedef class Block
{
public:

	// defines various block properties and behaviors
	enum BlockType : unsigned char // upgrade when over 256 block types
	{
		bAir = 0, // default type
		bStone,
		bDirt,
		bMetal,
		bGrass,
		bSand,
		bSnow,
		bWater,
		bOakWood,
		bOakLeaves,

		bCount
	};
	
	Block(BlockType t = bAir, unsigned char w = 0, unsigned char l = 0)
		: type_(t), writeStrength_(w), lightValue_(l) {}

	inline BlockType GetType() const { return type_; }
	inline unsigned char WriteStrength() const { return writeStrength_; }
	inline void SetType(BlockType ty, unsigned char write) { type_ = ty; writeStrength_ = write; }

	static const std::vector<BlockProperties> PropertiesTable;

private:
	BlockType type_ : 8;
	unsigned char lightValue_ : 4;

	// If the block was placed by the player or generated as part of a prefab,
	// this will be true. Used to determine whether to write to a block during
	// chunk generation. If this is true, then the block will not be modified
	// during TERRAIN generation (prefabs will overwrite the value).

	// this should be a char(?) called writeStrength instead to allow varying levels of written-ness
	unsigned char writeStrength_ : 4;
}Block, *BlockPtr;
#pragma pack(pop)

glm::ivec3 stretch(int index, int w, int h);