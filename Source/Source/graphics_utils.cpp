#include "stdafx.h"

namespace Utils
{
	void DeleteVAO(GLuint vao)
	{
		GLint max_vtx_attrib = 0;
		GLuint buffer_object;

		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vtx_attrib);
		glBindVertexArray(vao);

		for (int i = 0; i < max_vtx_attrib; ++i)
		{
			glGetVertexAttribIuiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &buffer_object);
			if (buffer_object > 0)
			{
				glDeleteBuffers(1, &buffer_object);
			}
		}

		glGetVertexArrayiv(vao, GL_ELEMENT_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*>(&buffer_object));

		if (buffer_object > 0)
		{
			glDeleteBuffers(1, &buffer_object);
		}

		glDeleteVertexArrays(1, &vao);
	}
}