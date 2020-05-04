#pragma once
#include "block.h"
#include "BitArray.h"

// uncompressed block storage for chunks
class ArrayBlockStorage
{
public:
	// num blocks
	ArrayBlockStorage(size_t size);
	~ArrayBlockStorage();
	ArrayBlockStorage(const ArrayBlockStorage&);
	ArrayBlockStorage& operator=(const ArrayBlockStorage&);

	Block& operator[](int index);
private:

	const size_t size_;
	Block* blocks = nullptr;
};


// compressed block storage
// lighting is uncompressed
class PaletteBlockStorage
{
public:
	PaletteBlockStorage(size_t size);
	~PaletteBlockStorage();
	PaletteBlockStorage(const PaletteBlockStorage&);
	PaletteBlockStorage& operator=(const PaletteBlockStorage&);

	void SetBlock(int index, Block);
	BlockType GetBlockType(int index);
	Light GetLight(int index);
private:
	struct PaletteEntry
	{
		int refcount = 0;
		BlockType type;
	};

	const size_t size_;
	BitArray data_;
	std::vector<PaletteEntry> palette_;
	size_t paletteEntryLength_ = 1;

};

#include "BlockStorage.inl"