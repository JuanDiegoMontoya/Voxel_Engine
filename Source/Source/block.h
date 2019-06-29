#pragma once
#include "level.h"
#include "pipeline.h"

// a 1x1x1 cube
typedef class Block
{
public:

	// defines various block properties and behaviors
	enum BlockType : unsigned char
	{
		bAir = 0, // default type
		bStone,
		bDirt,

		bCount
	};

	Block(BlockType ty = bAir) : _type(ty) {}

	inline BlockType GetType() { return _type; }
	inline void SetType(BlockType ty) { _type = ty; }

	inline bool IsEnabled() { return _enabled; }
	inline void SetEnabled(bool e) { _enabled = e; }

private:
	// all block data
	bool _enabled = true;
	BlockType _type = bAir;

}Block, *BlockPtr;

glm::ivec3 stretch(int index, int w, int h);