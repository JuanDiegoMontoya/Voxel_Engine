#pragma once
#include "vendor/ctpl_stl.h"

// Handles the bulk creation of chunks through threading and messaging.
// Does not have anything to do with building chunk meshes!
class ChunkLoadManager
{
public:
	ChunkLoadManager() { init(); }

	void Update();

private:
	void init();
	void sort(); // insertion sort (distance from camera)

	std::vector<ChunkPtr> genList_;
	ctpl::thread_pool pool_; // resources
};