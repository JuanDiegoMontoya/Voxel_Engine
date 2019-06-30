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

	Block(glm::vec3 p = glm::vec3(0), glm::vec4 c = glm::vec4(1), BlockType t = bAir)
		: clr(c), pos_(p), type_(t)
	{
		count_++;
		//update();
	}

	// positions, texcoords
	glm::vec4 clr = glm::vec4(1);

	inline const glm::mat4 GetModel()
	{
		return glm::translate(glm::mat4(1.f), glm::vec3(pos_));
	}

	inline void SetPos(glm::ivec3 p) 
	{ 
		pos_ = p;
		//update();
	}

	inline const glm::ivec3& GetPos() const { return pos_; }
	inline glm::vec3 GetMax() const
	{
		return glm::vec3(pos_) + .5f;
	}
	inline glm::vec3 GetMin() const
	{
		return glm::vec3(pos_) - .5f;
	}

	// cull block if all sides are invisible
	inline bool CheckCulled()
	{
		// not culled if max position (also prevents out of array error)
		if (pos_.x == 0 || pos_.y == 0 || pos_.z == 0 ||
			  pos_.x == 99 || pos_.y == 99 || pos_.z == 99)
			return false;

		// if any neighboring blocks are transparent, block isn't culled
		if (blocksarr_[ID3D(pos_.x, pos_.y, pos_.z + 1, 100, 100)].GetType() == bAir)
			return false;
		if (blocksarr_[ID3D(pos_.x, pos_.y, pos_.z - 1, 100, 100)].GetType() == bAir)
			return false;
		if (blocksarr_[ID3D(pos_.x, pos_.y + 1, pos_.z, 100, 100)].GetType() == bAir)
			return false;
		if (blocksarr_[ID3D(pos_.x, pos_.y - 1, pos_.z, 100, 100)].GetType() == bAir)
			return false;
		if (blocksarr_[ID3D(pos_.x + 1, pos_.y, pos_.z, 100, 100)].GetType() == bAir)
			return false;
		if (blocksarr_[ID3D(pos_.x - 1, pos_.y, pos_.z, 100, 100)].GetType() == bAir)
			return false;
		return true; // is culled
	}

	inline BlockType GetType() { return type_; }
	inline void SetType(BlockType ty) { type_ = ty; }
	inline bool IsCulled() { return isCulled_; }

	static unsigned count_;
	static Block blocksarr_[100 * 100 * 100]; // one million positions

	inline void Update()
	{
		//if (updated_)
		{
			isCulled_ = CheckCulled();
			updateList_->push_back(ID3D(pos_.x, pos_.y, pos_.z, 100, 100)); // notify renderer that a model has changed
			updated_ = false;
		}
	}

private:
	glm::ivec3 pos_; // position on grid (not necessary with a grid)
	bool isCulled_ = false;
	bool updated_ = true;
	BlockType type_ = bAir;

	// adds this block to update list if it hasn't been added this frame

	static std::vector<unsigned>* updateList_; // const screws it up
}Block, *BlockPtr;

glm::ivec3 stretch(int index, int w, int h);