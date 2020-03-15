#pragma once
#include "chunk_manager.h"
#include "block.h"
#include "camera.h"
//#include "game.h"
#include "game_object.h"
#include "sun.h"
//#include "render.h"
#include "hud.h"
#include "FixedQueue.h"

//class Camera;
typedef struct Chunk* ChunkPtr;
//typedef class Block* BlockPtr;
//typedef class Sun;

namespace World
{
	void Init();
	void Shutdown();
	void Update(float dt);
	void CheckCollision();
	void CheckInteraction();

	// unconditionally updates a block at a position
	void UpdateBlockAt(glm::ivec3 wpos, Block bl);
	void GenerateBlockAt(glm::ivec3 wpos, Block b); // updates a block at a position IF it isn't written yet
	void GenerateBlockAtCheap(glm::ivec3 wpos, Block b);
	Block GetBlockAt(glm::ivec3 wpos);

	inline void SetBgColor(glm::vec3 c) { bgColor_ = c; }

	//inline GamePtr Game() { return game_; }
	//inline const std::vector<GameObjectPtr>& GetObjects() const { return objects_; }
	//inline const glm::vec3& GetBgColor() const { return bgColor_; }
	//inline const Sun& GetSun() const { return sun_; }


	//private
	void checkBlockPlacement();
	void checkBlockDestruction();
	void checkBlockPick();


	// "private" variables
	 
	inline ChunkManager chunkManager_;
	//inline Renderer renderer_;
	inline HUD hud_;
	 
	inline std::string name_; // name of file
	//inline GamePtr game_;
	//inline std::vector<Camera*> cameras_;			 // all cameras in the scene
	inline std::vector<GameObjectPtr> objects_; // all game objects in the scene
	inline glm::vec3 bgColor_ = glm::vec3(.529f, .808f, .922f); // sky blue
	
	inline Sun sun_;
	inline bool doCollisionTick = true;
	// debug
	inline int debugCascadeQuad = 0;
}