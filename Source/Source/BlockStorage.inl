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





inline PaletteBlockStorage::PaletteBlockStorage(size_t chunkSize)
	: size_(chunkSize * chunkSize * chunkSize), data_(size_ * paletteEntryLength_)
{}

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
