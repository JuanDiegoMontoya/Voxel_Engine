#include "stdafx.h"

#include "ibo.h"

IBO::IBO(const GLubyte* data, unsigned int count) : count_(count)
{
	glGenBuffers(1, &rendererID_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererID_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLubyte), data, GL_STATIC_DRAW);
}

IBO::~IBO()
{
	glDeleteBuffers(1, &rendererID_);
}

void IBO::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererID_);
}

void IBO::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
