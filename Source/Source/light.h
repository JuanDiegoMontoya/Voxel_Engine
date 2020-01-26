#pragma once
#include "math_utils.h"

// describes the lighting level at a point in space, NOT the properties of a light emitter
typedef class Light
{
public:
	Light(glm::u8vec4 L = { 0, 0, 0, 0 }) { Set(L); }
	Light& operator=(const Light& rhs) { this->raw_ = rhs.raw_; return *this; }

	uint16_t& Raw() { return raw_; }

	glm::u8vec4 Get() { return { GetR(), GetG(), GetB(), GetS() }; }
	uint8_t GetR() { return raw_ >> 12; }
	uint8_t GetG() { return (raw_ >> 8) & 0b1111; }
	uint8_t GetB() { return (raw_ >> 4) & 0b1111; }
	uint8_t GetS() { return raw_ & 0b1111; }

	void Set(glm::u8vec4 L) { SetR(L.r); SetG(L.g); SetB(L.b); SetS(L.a); }
	void SetR(uint8_t r) { raw_ = (raw_ & 0x0FFF) | (r << 12); }
	void SetG(uint8_t g) { raw_ = (raw_ & 0xF0FF) | (g << 8); }
	void SetB(uint8_t b) { raw_ = (raw_ & 0xFF0F) | (b << 4); }
	void SetS(uint8_t s) { raw_ = (raw_ & 0xFFF0) | s; }
private:
	// 4 bits each of: red, green, blue, and sunlight
	uint16_t raw_;
}Light, *LightPtr;