#pragma once
#include "stdafx.h"

struct VAOData
{
	GLuint vao;
	GLuint count;
	VAOData(GLuint vao_, GLuint count_) : vao(vao_), count(count_) {}
	VAOData() : vao(0), count(0) {}
};