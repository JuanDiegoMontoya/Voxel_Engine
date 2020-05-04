#pragma once
#include "BlockStorage.h"

inline ArrayBlockStorage::ArrayBlockStorage(size_t chunkSize) : chunkSize_(chunkSize)
{
	blocks = new Block[chunkSize_ * chunkSize_ * chunkSize_];
}

inline ArrayBlockStorage::~ArrayBlockStorage()
{
	delete blocks;
}

inline ArrayBlockStorage::ArrayBlockStorage(const ArrayBlockStorage& other)
	: chunkSize_(other.chunkSize_)
{
	this->blocks = new Block[chunkSize_ * chunkSize_ * chunkSize_];
	*this = other;
}

inline ArrayBlockStorage& ArrayBlockStorage::operator=(const ArrayBlockStorage& other)
{
	ASSERT(this->chunkSize_ == other.chunkSize_);
	std::memcpy(this->blocks, other.blocks, chunkSize_ * chunkSize_ * chunkSize_);
	return *this;
}

inline Block& ArrayBlockStorage::operator[](int index)
{
	return blocks[index];
}
