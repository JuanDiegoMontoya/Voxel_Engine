#pragma once
#include "level.h"
#include "pipeline.h"

//typedef class Level* LevelPtr;

// a 1x1x1 cube
typedef class Block
{
public:
	Block(unsigned d) : _id(_count) { _count++; }

	// positions, texcoords
	std::vector<GLfloat> vertices;
	glm::vec4 clr = glm::vec4(1);
	std::vector<GLfloat> GetVertices();

	inline const glm::mat4& GetModel()
	{
		if (_dirtyModel)
		{
			_model = glm::translate(glm::mat4(1.f), glm::vec3(_pos));
			//_updated->push_back(_id);
			_dirtyModel = false;
		}
		return _model;
	}

	inline void SetPos(glm::ivec3 p) 
	{ 
		_pos = p;
		_dirtyModel = true;
		_updated->push_back(ID3D(_pos.x, _pos.y, _pos.z, 100, 100)); // notify renderer that a model has changed
	}

	inline const glm::ivec3& GetPos() const { return _pos; }
	inline glm::vec3 GetMax() const
	{
		return glm::vec3(_pos) + .5f;
	}
	inline glm::vec3 GetMin() const
	{
		return glm::vec3(_pos) - .5f;
	}

	void Update(float dt);

	inline bool CheckCulled(LevelPtr level)
	{
		BlockPtr* blocks = level->_blocksarr;
		if (!blocks[ID3D(_pos.x, _pos.y, _pos.z + 1, 100, 100)])
			return false;
		if (!blocks[ID3D(_pos.x, _pos.y, _pos.z - 1, 100, 100)])
			return false;
		if (!blocks[ID3D(_pos.x, _pos.y + 1, _pos.z, 100, 100)])
			return false;
		if (!blocks[ID3D(_pos.x, _pos.y - 1, _pos.z, 100, 100)])
			return false;
		if (!blocks[ID3D(_pos.x + 1, _pos.y, _pos.z, 100, 100)])
			return false;
		if (!blocks[ID3D(_pos.x - 1, _pos.y, _pos.z, 100, 100)])
			return false;
		return true; // is culled
	}

private:
	glm::ivec3 _pos; // position on grid
	glm::mat4 _model = glm::mat4(1.f); // a translation matrix
	bool _dirtyModel = true;
	bool _isCulled = false;
	const unsigned _id; // global unique identifier

	static unsigned _count;
	static std::vector<unsigned>* _updated; // const screws it up
}Block, *BlockPtr;