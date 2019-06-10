#include "stdafx.h"

#include "ibo.h"

IBO::IBO(const unsigned int* data, unsigned int count) : _count(count)
{
	glGenBuffers(1, &_rendererID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), data, GL_STATIC_DRAW);
}

IBO::~IBO()
{
	glDeleteBuffers(1, &_rendererID);
}

void IBO::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererID);
}

void IBO::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
