#include "stdafx.h"

#include "vbo.h"

VBO::VBO(const void * data, unsigned int size)
{
		glGenBuffers(1, &_rendererID);
		glBindBuffer(GL_ARRAY_BUFFER, _rendererID);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	//Create(data, size);
}

VBO::~VBO()
{
	glDeleteBuffers(1, &_rendererID);
}

//void VBO::Create(const void* data, unsigned int size)
//{
//	glGenBuffers(1, &_rendererID);
//	glBindBuffer(GL_ARRAY_BUFFER, _rendererID);
//	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
//}

void VBO::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, _rendererID);
}

void VBO::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
