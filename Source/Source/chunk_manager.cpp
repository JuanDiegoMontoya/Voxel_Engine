#include "stdafx.h"
#include "chunk.h"
#include "level.h"
#include "chunk_manager.h"

ChunkManager::ChunkManager()
{
	loadDistance_ = 0;
	unloadLeniency_ = 0;
	maxLoadPerFrame_ = 0;
}
