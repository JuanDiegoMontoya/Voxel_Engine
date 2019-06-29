#pragma once

#include "graphics_utils.h"

class VBO
{
public:
	VBO(const void* data, unsigned int size, GLenum drawmode = GL_STATIC_DRAW);
	~VBO();

	// will probably cause epic errors if called multiple times
	//void Create(const void* data, unsigned int size);
	void Bind() const;
	void Unbind() const;

private:
	GLuint rendererID_;
};