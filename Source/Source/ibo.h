#pragma once

#include "graphics_utils.h"

class IBO
{
public:
	IBO(const GLubyte* data, unsigned int count);
	~IBO();

	void Bind() const;
	void Unbind() const;

	inline GLuint GetCount() const { return count_; }

private:
	GLuint rendererID_;
	GLuint count_;
};