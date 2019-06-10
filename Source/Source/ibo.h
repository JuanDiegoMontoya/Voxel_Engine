#pragma once

#include "graphics_utils.h"

class IBO
{
public:
	IBO(const unsigned int* data, unsigned int count);
	~IBO();

	void Bind() const;
	void Unbind() const;

	inline GLuint GetCount() const { return _count; }

private:
	GLuint _rendererID;
	GLuint _count;
};