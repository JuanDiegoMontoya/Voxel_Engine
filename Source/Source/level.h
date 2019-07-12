#pragma once
#include "game.h"
#include "game_object.h"
#include "camera.h"
#include "sun.h"

typedef struct Chunk* ChunkPtr;
typedef class Block* BlockPtr;
//typedef class Sun;

typedef class Level
{
public:
	friend class Block;

	Level(std::string name); // will load from file in the future (maybe)
	~Level();

	void Init();
	void Update(float dt);
	void Draw();
	void DrawNormal();
	void DrawShadows();
	void DrawDebug();
	void CheckCollision();
	void CheckInteraction();
	void ProcessUpdatedChunks();

	inline void SetBgColor(glm::vec3 c) { bgColor_ = c; }

	inline GamePtr Game() { return game_; }
	inline const std::vector<GameObjectPtr>& GetObjects() const { return objects_; }
	inline const glm::vec3& GetBgColor() const { return bgColor_; }
	inline const Sun& GetSun() const { return sun_; }

	friend class Game;
private:
	std::vector<ChunkPtr> updatedChunks_;
	std::string name_; // name of file
	GamePtr game_;
	std::vector<Camera*> cameras_;			 // all cameras in the scene
	std::vector<GameObjectPtr> objects_; // all game objects in the scene
	glm::vec3 bgColor_ = glm::vec3(53.f / 255.f, 81.f / 255.f, 98.f / 255.f);
	
	Sun sun_;
	
	bool activeCursor = false;

	//https://www.reddit.com/r/VoxelGameDev/comments/2t1kkh/best_method_of_chunk_management_in_3d/
	//https://www.reddit.com/r/VoxelGameDev/comments/b6bgu8/voxel_chunk_management_c_opengl/
	//std::unordered_map<glm::ivec3, ChunkPtr> activechunks_;

	void checkBlockPlacement();
	void checkBlockDestruction();

}Level, *LevelPtr;

void renderAxisIndicators();
void renderQuad();