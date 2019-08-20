#pragma once
#include "vendor/ctpl_stl.h"
#include <atomic>

typedef struct Chunk* ChunkPtr;
typedef class Level* LevelPtr;

// Handles the bulk creation of chunks through threading and messaging.
// Does not have anything to do with building chunk meshes!
class ChunkLoadManager
{
public:
	ChunkLoadManager(LevelPtr l = nullptr)
	: level_(l) { init(); }

	void Update();
	void Push(ChunkPtr c);

private:
	void init();
	void sort(); // insertion sort by distance from camera
	void cull();
	void task(int id);

	// returns true if a's distance from camPos is larger than b's
	bool greater(ChunkPtr a, ChunkPtr b, const glm::vec3& camPos);

	LevelPtr level_;

	// <is being processed + the chunk>
	std::vector<std::pair<bool, ChunkPtr>> genList_;
	ctpl::thread_pool pool_; // resources
};