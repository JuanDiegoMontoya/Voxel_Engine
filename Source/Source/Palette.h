#pragma once
#include "BitArray.h"
#include <vector>

// fixed-size array optimized for space
template<typename T, unsigned _Size>
class Palette
{
public:
	Palette();
	~Palette();
	Palette(const Palette&);
	Palette& operator=(const Palette&);

	void SetVal(int index, T);
	T GetVal(int index) const;

private:
	struct PaletteEntry
	{
		T type;
		int refcount = 0;
	};

	unsigned newPaletteEntry();
	void growPalette();
	void fitPalette();

	BitArray data_;
	std::vector<PaletteEntry> palette_;
	unsigned paletteEntryLength_ = 1;
};


// thread-safe variation of the palette
template<typename T, unsigned _Size>
class ConcurrentPalette : public Palette<T, _Size>
{
public:

	void SetVal(int index, T val)
	{
		std::lock_guard w(mtx);
		Palette<T, _Size>::SetVal(index, val);
	}

	T GetVal(int index) const
	{
		std::shared_lock r(mtx);
		return Palette<T, _Size>::GetVal(index);
	}

private:
	// writes are exclusive, but not reads
	mutable std::shared_mutex mtx;
};

#include "Palette.inl"