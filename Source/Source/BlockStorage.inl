#pragma once
#include "BlockStorage.h"

inline ArrayBlockStorage::ArrayBlockStorage(size_t size) : size_(size)
{
	blocks_ = new Block[size_];
}

inline ArrayBlockStorage::~ArrayBlockStorage()
{
	delete blocks_;
}

inline ArrayBlockStorage::ArrayBlockStorage(const ArrayBlockStorage& other)
	: size_(other.size_)
{
	this->blocks_ = new Block[size_];
	*this = other;
}

inline ArrayBlockStorage& ArrayBlockStorage::operator=(const ArrayBlockStorage& other)
{
	ASSERT(this->size_ == other.size_);
	//std::memcpy(this->blocks_, other.blocks_, sizeof(Block) * size_);
	std::copy(other.blocks_, other.blocks_ + size_, this->blocks_);
	return *this;
}

inline Block& ArrayBlockStorage::operator[](int index)
{
	return blocks_[index];
}

inline Block& ArrayBlockStorage::GetBlockRef(int index)
{
	return blocks_[index];
}

inline Block ArrayBlockStorage::GetBlock(int index)
{
	return blocks_[index];
}

inline BlockType ArrayBlockStorage::GetBlockType(int index)
{
	return blocks_[index].GetType();
}

inline void ArrayBlockStorage::SetBlock(int index, BlockType type)
{
	blocks_[index].SetType(type);
}

inline void ArrayBlockStorage::SetLight(int index, Light light)
{
	blocks_[index].GetLightRef() = light;
}

inline Light ArrayBlockStorage::GetLight(int index)
{
	return blocks_[index].GetLight();
}





inline PaletteBlockStorage::PaletteBlockStorage(size_t size)
	: size_(size)
{
	data_.Resize(size_ * paletteEntryLength_);
	// # of block choices corresponds to # of bits required for palette entry
	// 2^paletteEntryLength choices
	palette_.resize(1u << paletteEntryLength_);
	palette_[0].refcount = size_;
}

inline PaletteBlockStorage::~PaletteBlockStorage()
{}

inline PaletteBlockStorage::PaletteBlockStorage(const PaletteBlockStorage& other)
	: size_(other.size_)
{
	*this = other;
}

inline PaletteBlockStorage& PaletteBlockStorage::operator=(const PaletteBlockStorage& other)
{
	ASSERT(this->size_ == other.size_);
	this->data_ = other.data_;
	this->palette_ = other.palette_;
	this->paletteEntryLength_ = other.paletteEntryLength_;
	return *this;
}

inline void PaletteBlockStorage::SetBlock(int index, BlockType type)
{
	//std::shared_lock lk(mtx);
	//std::lock_guard lk(mtx);
	//lk.lock();
	mtx.lock();
	//std::cout << "LOCKED\n";

	unsigned paletteIndex = data_.GetSequence(index * paletteEntryLength_, paletteEntryLength_);
	auto& current = palette_[paletteIndex]; // compiler forces me to make this auto

	// remove reference to block that is already there
	current.refcount--;

	// check if block type is already in palette
	int replaceIndex = -1;
	for (int i = 0; i < palette_.size(); i++)
	{
		if (palette_[i].type == type)
		{
			replaceIndex = i;
			// use existing palette entry
			data_.SetSequence(index * paletteEntryLength_, paletteEntryLength_, unsigned(replaceIndex));
			palette_[replaceIndex].refcount++;
			mtx.unlock();
			return;
		}
	}

	// check if palette entry of block we just removed is empty
	if (current.refcount == 0)
	{
		current.type = type;
		current.refcount = 1;
		mtx.unlock();
		return;
	}

	// we need a new palette entry, dawg
	unsigned newEntry = newPaletteEntry();
	palette_[newEntry] = { type, 1 };
	data_.SetSequence(index * paletteEntryLength_, paletteEntryLength_, newEntry);

	//std::cout << "UNLOCKED\n";
	mtx.unlock();
}

inline Block PaletteBlockStorage::GetBlock(int index)
{
	return Block(GetBlockType(index), GetLight(index));
}

inline BlockType PaletteBlockStorage::GetBlockType(int index)
{
	mtx.lock_shared();
	unsigned paletteIndex = data_.GetSequence(index * paletteEntryLength_, paletteEntryLength_);
	BlockType ret = palette_[paletteIndex].type;
	mtx.unlock_shared();
	return ret;
}

inline void PaletteBlockStorage::SetLight(int index, Light light)
{
	// TODO: this
}

inline Light PaletteBlockStorage::GetLight(int index)
{
	// TODO: this
	return Light();
}

inline unsigned PaletteBlockStorage::newPaletteEntry()
{
	while (1)
	{
		// find index of free palette entry
		for (int i = 0; i < palette_.size(); i++)
			if (palette_[i].refcount == 0) // empty or uninitialized entry
				return i;

		// grow palette if no free entry
		growPalette();
	}
}

inline void PaletteBlockStorage::growPalette()
{
	// decode indices (index into palette_)
	std::vector<unsigned> indices;
	indices.resize(size_);
	for (int i = 0; i < size_; i++)
		indices[i] = data_.GetSequence(i * paletteEntryLength_, paletteEntryLength_);

	// double length of palette
	//paletteEntryLength_ <<= 1;
	paletteEntryLength_++;
	palette_.resize(1u << paletteEntryLength_);

	// increase length of bitset to accommodate extra bit
	data_.Resize(size_ * paletteEntryLength_);

	// encode previous indices with extended length
	for (int i = 0; i < indices.size(); i++)
		data_.SetSequence(i * paletteEntryLength_, paletteEntryLength_, indices[i]);
}
