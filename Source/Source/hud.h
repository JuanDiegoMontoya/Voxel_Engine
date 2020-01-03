#pragma once
#include "block.h"

// renders stuff directly to the screen
class HUD
{
public:
	void Update();
	BlockType selected_ = BlockType::bError;
private:
};