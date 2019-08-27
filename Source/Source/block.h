#pragma once
//#include "level.h"
//#include "pipeline.h"
#include "serialize.h"

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
//#pragma pack(push, 1)
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
		bError,
		bDryGrass,

		bCount
	};
	
	Block(BlockType t = bAir, unsigned char w = 0, unsigned char l = 0)
		: type_(t)
	{
		SetWriteStrength(w);
		SetLightValue(l);
	}

	// Getters
	BlockType GetType() const { return type_; }
	unsigned char WriteStrength() const { return (wlValues_ & 0xF0) >> 4; }
	unsigned char LightValue() const { return wlValues_ & 0x0F; }

	// Setters
	void SetType(BlockType ty, unsigned char write) { type_ = ty; SetWriteStrength(write); }
	void SetWriteStrength(unsigned char w)
	{
		ASSERT(w <= UCHAR_MAX / 2);
		wlValues_ = (wlValues_ & 0x0F) | (w << 4);
	}
	void SetLightValue(unsigned char l)
	{
		ASSERT(l <= UCHAR_MAX / 2);
		wlValues_ = (wlValues_ & 0xF0) | (l);
	}

	// Serialization
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(type_, wlValues_);
	}

	static const std::vector<BlockProperties> PropertiesTable;
private:
	BlockType type_;

	// left 4 bits = writeStrength; right 4 bits = lightValue
	unsigned char wlValues_;
}Block, *BlockPtr;
//#pragma pack(pop)

glm::ivec3 stretch(int index, int w, int h);