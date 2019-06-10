#pragma once

#include "stdafx.h"

class Texture
{
public:
	Texture(const std::string& path);
	~Texture();

	void Bind(GLuint slot = 0) const;
	void Unbind() const;

	inline GLint GetWidth() const { return _width; }
	inline GLint GetHeight() const { return _height; }
	inline GLint GetID() const { return _rendererID; }

	inline void SetType(std::string ty) { _type = ty; }
	inline const std::string& GetType() const { return _type; }

private:
	GLuint _rendererID;
	std::string _filepath;
	std::string _type;
	GLubyte* _localbuffer;
	GLint _width, _height, _BPP;
	static const char* _texture_dir;
};