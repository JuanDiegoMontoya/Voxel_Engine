#pragma once
#include "block.h"

// for chunks
class ArrayBlockStorage
{
public:
	// should be power of 2 (i.e. 2^5)
	ArrayBlockStorage(size_t chunkSize);
	~ArrayBlockStorage();
	ArrayBlockStorage(const ArrayBlockStorage&);
	ArrayBlockStorage& operator=(const ArrayBlockStorage&);

	Block& operator[](int index);
private:

	const size_t chunkSize_;
	Block* blocks = nullptr;
};


class PaletteBlockStorage
{
public:
	

private:
	struct PaletteEntry
	{
		int refcount = 0;
		BlockType type;
	};

	const size_t size;
	std::vector<bool> data;
	PaletteEntry* palette;
};

#include "BlockStorage.inl"