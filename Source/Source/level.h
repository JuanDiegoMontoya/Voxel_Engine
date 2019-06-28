#pragma once
#include "game.h"
#include "game_object.h"
#include "camera.h"

typedef struct Chunk* ChunkPtr;
typedef class Block* BlockPtr;

typedef class Level
{
public:
	friend class Block;

	Level(std::string name); // will load from file in the future (maybe)
	~Level();

	void Init();
	void Update(float dt);
	void CheckCollision();
	void CheckInteraction();

	inline void SetBgColor(glm::vec3 c) { _bgColor = c; }

	inline GamePtr Game() { return _game; }
	inline const std::vector<GameObjectPtr>& GetObjects() const { return _objects; }
	inline const std::vector<BlockPtr>& GetBlocks() const { return _blocks; }
	inline const BlockPtr* GetBlocksArray() { return _blocksarr; }
	inline const glm::vec3& GetBgColor() const { return _bgColor; }
private:
	std::string _name; // name of file
	GamePtr _game;
	std::vector<Camera*> _cameras;			 // all cameras in the scene
	std::vector<GameObjectPtr> _objects; // all game objects in the scene
	glm::vec3 _bgColor = glm::vec3(53.f / 255.f, 81.f / 255.f, 98.f / 255.f);
	
	//https://www.reddit.com/r/VoxelGameDev/comments/2t1kkh/best_method_of_chunk_management_in_3d/
	//https://www.reddit.com/r/VoxelGameDev/comments/b6bgu8/voxel_chunk_management_c_opengl/
	//std::unordered_map<glm::ivec3, ChunkPtr> _activechunks;

	std::vector<BlockPtr> _blocks; // TEMPORARY SOLUTION
	BlockPtr _blocksarr[100 * 100 * 100]; // one million positions
	BlockPtr THE_CHOSEN_ONE;
	int CHOSEN_POS;
}Level, *LevelPtr;