#pragma once
#include "chunk_manager.h"
#include "block.h"
#include "camera.h"
#include "game.h"
#include "game_object.h"
#include "sun.h"

//class Camera;
typedef struct Chunk* ChunkPtr;
//typedef class Block* BlockPtr;
//typedef class Sun;

typedef class Level
{
public:
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
	void UpdateBlockAt(glm::ivec3 wpos, Block::BlockType type);

	inline void SetBgColor(glm::vec3 c) { bgColor_ = c; }

	inline GamePtr Game() { return game_; }
	inline const std::vector<GameObjectPtr>& GetObjects() const { return objects_; }
	inline const glm::vec3& GetBgColor() const { return bgColor_; }
	inline const Sun& GetSun() const { return sun_; }

	friend class Game;
	friend class Block;
	friend class WorldGen;
private:
	ChunkManager chunkManager_;

	std::string name_; // name of file
	GamePtr game_;
	std::vector<Camera*> cameras_;			 // all cameras in the scene
	std::vector<GameObjectPtr> objects_; // all game objects in the scene
	glm::vec3 bgColor_ = glm::vec3(.529f, .808f, .922f); // sky blue
	
	Sun sun_;
	float renderdist_ = 500.f;
	float renderLeniency_ = 100.f;
	bool activeCursor = false;

	void checkBlockPlacement();
	void checkBlockDestruction();

	// debug
	int debugCascadeQuad = 0;
}Level, *LevelPtr;

void renderAxisIndicators();
void renderQuad();