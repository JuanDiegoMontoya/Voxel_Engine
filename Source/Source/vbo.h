#pragma once

#include "graphics_utils.h"

class VBO
{
public:
	VBO(const void* data, unsigned int size);
	~VBO();

	// will probably cause epic errors if called multiple times
	//void Create(const void* data, unsigned int size);
	void Bind() const;
	void Unbind() const;

private:
	GLuint _rendererID;
};