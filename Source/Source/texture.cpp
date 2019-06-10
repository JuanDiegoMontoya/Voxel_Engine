#include "stdafx.h"
#include "texture.h"
#include <stb_image.h>

const char* Texture::_texture_dir = "./resources/Textures/";

Texture::Texture(const std::string & path)
	: _rendererID(0), _filepath(path), _localbuffer(nullptr), _width(0), _height(0), _BPP(0)
{
	std::string realpath = _texture_dir + path;

	// top = 0
	stbi_set_flip_vertically_on_load(1);

	// 4 = rgba
	_localbuffer = stbi_load(realpath.c_str(), &_width, &_height, &_BPP, 4);

	if (!_localbuffer)
		std::cout << "Failed to load texture: " << path << std::endl;

	glGenTextures(1, &_rendererID);
	glBindTexture(GL_TEXTURE_2D, _rendererID);
	
	// these four parameters must be specified by us
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // x
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // y

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _localbuffer);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (_localbuffer)
		stbi_image_free(_localbuffer);
}

Texture::~Texture()
{
	glDeleteTextures(1, &_rendererID);
}

void Texture::Bind(GLuint slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, _rendererID);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
