#include "stdafx.h"
#include "component.h"
#include "ibo.h"
#include "vbo.h"
#include "vao.h"
#include "texture.h"
#include "pipeline.h"
#include "shader.h"

#include "render_data.h"

// generate a new set of stuff to use
// this method sucks (should instead be using a list of presets defined in render)
void RenderData::UseUntexturedBlockData()
{
	if (_vao)
		delete _vao;
	if (_vbo)
		delete _vbo;
	if (_ibo)
		delete _ibo;

	_vbo = new VBO(Render::cube_vertices, sizeof(Render::cube_vertices));
	_vao = new VAO();
	VBOlayout layout;
	layout.Push<GLfloat>(3);
	layout.Push<GLfloat>(3);
	layout.Push<GLfloat>(2);
	_vao->AddBuffer(*_vbo, layout);

	_shader = Shader::shaders["flat"];
}