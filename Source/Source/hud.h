#pragma once
#include "block.h"

// renders stuff directly to the screen
class HUD
{
public:
	void Update();
	Block::BlockType selected_ = Block::bError;
private:
};