#include "stdafx.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "render.h"

using namespace std;

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	setupMesh();
}

void Mesh::setupMesh()
{
	vao = new VAO();
	vbo = new VBO(&vertices[0], vertices.size() * sizeof(Vertex));

	VBOlayout layout;
	layout.Push<GLfloat>(3);
	//layout.Push<GLfloat>(3);
	layout.Push<GLfloat>(2);
	vao->AddBuffer(*vbo, layout);
	vbo->Unbind();
	ibo = new IBO(&indices[0], vertices.size());
	vao->Unbind();
	//glGenVertexArrays(1, &vAO);
	//glGenBuffers(1, &vBO);
	//glGenBuffers(1, &eBO);

	//glBindVertexArray(vAO);
	//glBindBuffer(GL_ARRAY_BUFFER, vBO);

	//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
	//	&indices[0], GL_STATIC_DRAW);

	//// positions
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	//// normals
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	//// texture coords
	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	//glBindVertexArray(0);
}
