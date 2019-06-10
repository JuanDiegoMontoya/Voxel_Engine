#pragma once
//#include "stdafx.h"

class VAO;
class IBO;
class Shader;

class Renderer
{
public:
	void Draw(const VAO& va, const IBO& ib, const Shader& shader);
	void DrawArrays(const VAO & va, GLuint count, const Shader & shader);
	void Clear();
	//void SetClearColor() const;

private:

};