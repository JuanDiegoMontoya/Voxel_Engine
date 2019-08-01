#include "stdafx.h"
#include "vbo.h"
#include "vao.h"
#include "ibo.h"
#include "shader.h"
#include "render.h"

// draws everything at once, I guess?
// this should be fine
void Renderer::DrawAll()
{
}

void Renderer::Clear()
{
	glClearColor(0, 0, 0, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawChunk(ChunkPtr chunk, shaderCB uniform_cb)
{
}

void Renderer::drawChunk(ChunkPtr chunk, shaderCB uniform_cb)
{
}

void Renderer::drawBillboard(VAO * vao, size_t count, shaderCB uniform_cb)
{
}
