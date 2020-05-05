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
	Block& GetBlockRef(int index);
	Block GetBlock(int index);
private:

	const size_t size_;
	Block* blocks_ = nullptr;
};


// https://www.reddit.com/r/VoxelGameDev/comments/9yu8qy/palettebased_compression_for_chunked_discrete/
// compressed block storage
// lighting is uncompressed
// can't really return references w/o doing crazy proxy class stuff
class PaletteBlockStorage
{
public:
	PaletteBlockStorage(size_t size);
	~PaletteBlockStorage();
	PaletteBlockStorage(const PaletteBlockStorage&);
	PaletteBlockStorage& operator=(const PaletteBlockStorage&);

	void SetBlock(int index, BlockType);
	Block GetBlock(int index);
	BlockType GetBlockType(int index);
	void SetLight(int index, Light);
	Light GetLight(int index);

private:
	struct PaletteEntry
	{
		BlockType type;
		int refcount = 0;
	};

	unsigned newPaletteEntry();
	void growPalette();

	const size_t size_;
	BitArray data_;
	std::vector<PaletteEntry> palette_;
	size_t paletteEntryLength_ = 1;

};

#include "BlockStorage.inl"