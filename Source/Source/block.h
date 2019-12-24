#pragma once
//#include "level.h"
//#include "pipeline.h"
#include "serialize.h"
#include "utilities.h"

// visual properties (for now)
struct BlockProperties
{
	BlockProperties(const char* n, float s, glm::vec4 c, glm::uvec4 e)
	: name(n), color(c), specular(s), invisible(c.a == 0), emittance(e) {}
	const char* name;
	float specular;					// shininess
	glm::vec4 color;				// diffuse color
	bool invisible;					// skip rendering if true
	glm::ucvec4 emittance;	// light
};

// a 1x1x1 cube
//#pragma pack(push, 1)
typedef class Block
{
public:

	// defines various block properties and behaviors
	enum BlockType : uint8_t // upgrade when over 256 block types
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
		bOLight,
		bRLight,
		bGLight,
		bBLight,
		bSmLight,

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
	const char* GetName() const { return Block::PropertiesTable[unsigned(type_)].name; }
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
		// TODO: serialize wlValues as well
		ar(type_, wlValues_);
	}

	static const std::vector<BlockProperties> PropertiesTable;
private:
	BlockType type_;

	// left 4 bits = writeStrength; right 4 bits = lightValue
	// the lighting information can be derived at runtime, but
	// the write strength should be serialized in the future
	uint8_t wlValues_;
}Block, *BlockPtr;
//#pragma pack(pop)