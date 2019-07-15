#pragma once
#include "level.h"
#include "pipeline.h"

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

		bCount
	};

	Block(BlockType t = bAir)	: type_(t)
	{
		count_++;
		//update();
	}

	// positions, texcoords
	//glm::vec4 clr = glm::vec4(1);

	//inline const glm::ivec3& GetPos() const { return pos_; }
	//inline glm::vec3 GetMax() const
	//{
	//	//return glm::vec3(pos_) + .5f;
	//}
	//inline glm::vec3 GetMin() const
	//{
	//	//return glm::vec3(pos_) - .5f;
	//}

	inline BlockType GetType() { return type_; }
	inline void SetType(BlockType ty) { type_ = ty; }

	static unsigned count_;
	static Block blocksarr_[100 * 100 * 100]; // one million positions

	inline void Update()
	{
		//if (updated_)
		{
			//updateList_->push_back(ID3D(pos_.x, pos_.y, pos_.z, 100, 100)); // notify renderer that a model has changed
			//updated_ = false;
		}
	}

private:
	//glm::ivec3 pos_; // position on grid (not necessary with a grid)
	//bool updated_ = true;
	BlockType type_ = bAir;

	// adds this block to update list if it hasn't been added this frame

	static std::vector<unsigned>* updateList_; // const screws it up
}Block, *BlockPtr;

glm::ivec3 stretch(int index, int w, int h);