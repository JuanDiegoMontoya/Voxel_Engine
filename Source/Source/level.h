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

	inline void SetBgColor(glm::vec3 c) { bgColor_ = c; }

	inline GamePtr Game() { return game_; }
	inline const std::vector<GameObjectPtr>& GetObjects() const { return objects_; }
	inline const glm::vec3& GetBgColor() const { return bgColor_; }
private:
	std::string name_; // name of file
	GamePtr game_;
	std::vector<Camera*> cameras_;			 // all cameras in the scene
	std::vector<GameObjectPtr> objects_; // all game objects in the scene
	glm::vec3 bgColor_ = glm::vec3(53.f / 255.f, 81.f / 255.f, 98.f / 255.f);
	
	//https://www.reddit.com/r/VoxelGameDev/comments/2t1kkh/best_method_of_chunk_management_in_3d/
	//https://www.reddit.com/r/VoxelGameDev/comments/b6bgu8/voxel_chunk_management_c_opengl/
	//std::unordered_map<glm::ivec3, ChunkPtr> activechunks_;

}Level, *LevelPtr;