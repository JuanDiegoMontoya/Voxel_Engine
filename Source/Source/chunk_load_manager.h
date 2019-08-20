#pragma once
#include "vendor/ctpl_stl.h"
//#include <atomic>

#define USE_MULTITHREADED_CHUNK_LOADER 0

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
	void SetCurrentLevel(LevelPtr level);

private:
	void init();
	void sort(); // insertion sort by distance from camera
	void cull();
	void task(int id, ChunkPtr c);

	// returns true if a's distance from camPos is larger than b's
	bool greater(ChunkPtr a, ChunkPtr b, const glm::vec3& camPos);

	LevelPtr level_;

	// <is being processed + the chunk>
	std::vector<ChunkPtr> genList_;
	ctpl::thread_pool pool_; // resources
};