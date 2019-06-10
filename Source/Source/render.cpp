#include "stdafx.h"
#include "vbo.h"
#include "vao.h"
#include "ibo.h"
#include "shader.h"
#include "render.h"

void Renderer::Draw(const VAO & va, const IBO & ib, const Shader & shader)
{
	shader.Use();
	va.Bind();
	ib.Bind();
	glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, 0);
}

void Renderer::DrawArrays(const VAO & va, GLuint count, const Shader & shader)
{
	shader.Use();
	va.Bind();
	glDrawArrays(GL_TRIANGLES, 0, count);
}

void Renderer::Clear()
{
	glClearColor(0, 0, 0, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
