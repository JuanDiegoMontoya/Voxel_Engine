#pragma once

#include "stdafx.h"

struct VBOElement
{
	GLuint type;
	GLuint count;
	GLuint normalized;

	static GLuint TypeSize(GLuint type)
	{
		switch (type)
		{
		case GL_FLOAT:				 return sizeof(GLfloat);
		case GL_UNSIGNED_INT:  return sizeof(GLuint);
		case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
		}

		ASSERT(false);
		return 0;
	}
};

class VBOlayout
{
public:
	VBOlayout() : _stride(0) {}

	inline GLuint GetStride() const { return _stride; }
	inline const std::vector<VBOElement>& GetElements() const { return _elements; }

	template<typename T>
	void Push(unsigned int count)
	{
		ASSERT_MSG(false, "Invalid type specified."); // crash
	}

	// template specializations for Push()
	template<>
	void Push<float>(unsigned int count)
	{
		_elements.push_back({ GL_FLOAT, count, GL_FALSE });
		_stride += count * VBOElement::TypeSize(GL_FLOAT);
	}

	template<>
	void Push<GLuint>(unsigned int count)
	{
		_elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		_stride += count * VBOElement::TypeSize(GL_UNSIGNED_INT);
	}

	template<>
	void Push<GLubyte>(unsigned int count)
	{
		_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		_stride += count * VBOElement::TypeSize(GL_UNSIGNED_BYTE);
	}

private:
	std::vector<VBOElement> _elements;
	GLuint _stride;
};