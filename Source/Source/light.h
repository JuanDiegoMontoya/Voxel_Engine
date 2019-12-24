#pragma once

namespace glm
{
	// r, g, b, and sunlight
	typedef glm::vec<2, unsigned char> ucvec2;
}

// describes the lighting level at a point in space, NOT the properties of a light emitter
typedef class Light
{
public:
	uint8_t GetR() { return raw_ >> 12; }
	uint8_t GetG() { return (raw_ >> 8) & 0b1111; }
	uint8_t GetB() { return (raw_ >> 4) & 0b1111; }
	uint8_t GetS() { return raw_ & 0b1111; }

	void SetR(uint8_t r) { raw_ = (raw_ & 0b0000111111111111) | r; }
	void SetG(uint8_t g) { raw_ = (raw_ & 0b1111000011111111) | g; }
	void SetB(uint8_t b) { raw_ = (raw_ & 0b1111111100001111) | b; }
	void SetS(uint8_t s) { raw_ = (raw_ & 0b1111111111110000) | s; }
private:
	// 4 bits each of: red, green, blue, and sunlight
	uint16_t raw_;
}Light, *LightPtr;