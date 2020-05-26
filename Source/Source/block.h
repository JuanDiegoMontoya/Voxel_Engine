#pragma once
#include "serialize.h"
#include "light.h"

// visual properties (for now)
struct BlockProperties
{
	BlockProperties(const char* n, glm::vec4 c, glm::uvec4 e, const char* tx = "<null>")
	: name(n), color(c), invisible(c.a == 0), emittance(e), texture(tx) {}
	const char* name;
	glm::vec4 color;        // diffuse color
	bool invisible;         // skip rendering if true
	glm::u8vec4 emittance;	// light
	const char* texture;    // path to texture
};


// defines various block properties and behaviors
enum class BlockType : uint16_t // upgrade when over 2^16 block types
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
	bBglass,

	bCount
};


typedef struct Block
{
public:
	
	Block(BlockType t = BlockType::bAir) : type_(t) {}
	Block(BlockType t, Light l) : type_(t), light_(l) {}

	// Getters
	BlockType GetType() const { return type_; }
	int GetTypei() const { return int(type_); }
	const char* GetName() const { return Block::PropertiesTable[unsigned(type_)].name; }
	Light& GetLightRef() { return light_; }
	Light GetLight() const { return light_; }

	// Setters
	void SetType(BlockType ty) { type_ = ty; }

	// Serialization
	template <class Archive>
	void serialize(Archive& ar)
	{
		uint8_t fake;
		ar(type_, fake);
	}

	static const std::vector<BlockProperties> PropertiesTable;
private:
	BlockType type_; // could probably shove extra data in this
	Light light_;

}Block, *BlockPtr;