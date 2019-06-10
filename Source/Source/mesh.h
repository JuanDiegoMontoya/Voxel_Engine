#pragma once
#include "shader.h"
#include "texture.h"
#include "vao.h"
#include "ibo.h"
#include "vbo.h"
#include "render.h"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	explicit Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	
	virtual void Draw(ShaderPtr shader) {};

protected:
	Mesh() = delete;

	//GLuint vAO, vBO, eBO;
	VAO* vao;
	VBO* vbo;
	IBO* ibo;

	Renderer renderer;
	bool indexed; // whether to use DrawElements or DrawArrays
	void setupMesh();
};