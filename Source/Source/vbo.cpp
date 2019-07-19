#include "stdafx.h"

#include "vbo.h"

VBO::VBO(const void * data, unsigned int size, GLenum drawmode)
{
		glGenBuffers(1, &rendererID_);
		glBindBuffer(GL_ARRAY_BUFFER, rendererID_);
		glBufferData(GL_ARRAY_BUFFER, size, data, drawmode);
		//ASSERT(rendererID_ < 10'000);
	//Create(data, size);
}

VBO::~VBO()
{
	glDeleteBuffers(1, &rendererID_);
}

//void VBO::Create(const void* data, unsigned int size)
//{
//	glGenBuffers(1, &rendererID_);
//	glBindBuffer(GL_ARRAY_BUFFER, rendererID_);
//	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
//}

void VBO::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, rendererID_);
}

void VBO::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
