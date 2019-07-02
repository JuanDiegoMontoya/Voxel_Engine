#include "stdafx.h"
#include "vbo.h"
#include "vao.h"
#include "chunk.h"
#include "block.h"
#include "camera.h"
#include "shader.h"
#include <mutex>

Concurrency::concurrent_unordered_map<glm::ivec3, Chunk*, Chunk::ivec3Hash> Chunk::chunks;

static std::mutex mtx;

Chunk::Chunk(bool active) : active_(active)
{
	std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
	lck.lock();
	vao_ = new VAO();
	lck.unlock();

	float r = Utils::get_random(0, 1);
	float g = Utils::get_random(0, 1);
	float b = Utils::get_random(0, 1);
	color = glm::vec4(r, g, b, 1.f);
}

Chunk::~Chunk()
{
	if (vao_)
		delete vao_;
	if (vbo_)
		delete vbo_;
}

void Chunk::Update()
{
	//// update all blocks within the chunk
	//for (int x = 0; x < CHUNK_SIZE; x++)
	//{
	//	for (int y = 0; y < CHUNK_SIZE; y++)
	//	{
	//		for (int z = 0; z < CHUNK_SIZE; z++)
	//		{
	//			blocks[ID3D(x, y, z, CHUNK_SIZE, CHUNK_SIZE)].Update();
	//		}
	//	}
	//}

	// build mesh after ensuring culled blocks are culled
	buildMesh();
}

void Chunk::Render()
{
	if (vbo_ && vao_)
	{
		glDisable(GL_BLEND);
		//glDisable(GL_CULL_FACE);
		vao_->Bind();
		vbo_->Bind();
		ShaderPtr currShader = Shader::shaders["chunk"];
		currShader->Use();
		currShader->setMat4("u_model", model_);
		currShader->setMat4("u_view", Render::GetCamera()->GetView());
		currShader->setMat4("u_proj", Render::GetCamera()->GetProj());

		glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
	}
}

void Chunk::BuildBuffers()
{
	vao_->Bind();
	if (vbo_)
		delete vbo_;
	vbo_ = new VBO(&vertices[0], sizeof(glm::vec3) * vertices.size(), GL_STATIC_DRAW);
	vbo_->Bind();
	vertexCount_ = vertices.size();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0); // screenpos
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3)); // color
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 0);
	vao_->Unbind();
	vbo_->Unbind();
	vertices.clear();
}

void Chunk::buildMesh()
{
	Camera* cam = Render::GetCamera();

	vertices.reserve(CHUNK_SIZE * CHUNK_SIZE * 6 * 3); // one entire side of a chunk (assumed flat)
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				// skip fully transparent blocks
				if (blocks[ID3D(x, y, z, CHUNK_SIZE, CHUNK_SIZE)].GetType() == Block::bAir)
					continue;

				// check if each face would be obscured, and adds the ones that aren't to the vbo
				// obscured IF side is adjacent to opaque block
				// NOT obscured if side is adjacent to nothing or transparent block
				std::vector<glm::vec3> temp;

				// back
				temp = buildSingleBlockFace(glm::vec3(x, y, z - 1), 0, 18, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());

				// front
				temp = buildSingleBlockFace(glm::vec3(x, y, z + 1), 18, 36, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());

				// left
				temp = buildSingleBlockFace(glm::vec3(x - 1, y, z), 36, 54, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());

				// right
				temp = buildSingleBlockFace(glm::vec3(x + 1, y, z), 54, 72, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());

				// bottom
				temp = buildSingleBlockFace(glm::vec3(x, y - 1, z), 72, 90, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());

				// top
				temp = buildSingleBlockFace(glm::vec3(x, y + 1, z), 90, 108, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());
			}
		}
	}
	// "buildbuffers" would normally happen here
	// 'vertices' is stored until "buildbuffers" is called
}

std::vector<glm::vec3> Chunk::buildSingleBlockFace(glm::ivec3 near, int low, int high, int x, int y, int z)
{
	std::vector<glm::vec3> quad;
	//quad.reserve(6);
	
	localpos nearblock = worldBlockToLocalPos(chunkBlockToWorldPos(near));

	// critical section (probably)
	//std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
	//lck.lock();
	if (!chunks[nearblock.chunk_pos])
		goto GenQuad;
	if (!chunks[nearblock.chunk_pos]->active_)
		goto GenQuad;
	if (chunks[nearblock.chunk_pos]->At(nearblock.block_pos).GetType() != Block::bAir)
		return quad;
	//lck.unlock();

	GenQuad:
	// transform the vertices relative to the chunk
	// (the full world transformation will be completed in a shader)
	glm::mat4 localTransform = glm::translate(glm::mat4(1.f),
		Utils::mapToRange(glm::vec3(x, y, z), 0.f, (float)CHUNK_SIZE, -(float)CHUNK_SIZE / 2.0f, (float)CHUNK_SIZE/ 2.0f)); // scaled

	//glm::mat4 localTransform = glm::translate(glm::mat4(1.f), glm::vec3(x, y, z)); // non-scaled
	// add a random color to each quad
	float r = Utils::get_random_r(0, 1);
	float g = Utils::get_random_r(0, 1);
	float b = Utils::get_random_r(0, 1);
	glm::vec3 colory(r, g, b);

	for (int i = low; i < high; i += 3)
	{
		glm::vec4 tri(Render::cube_vertices[i], Render::cube_vertices[i + 1], Render::cube_vertices[i + 2], 1.f);
		quad.push_back(localTransform * tri);
		quad.push_back(colory);
	}

	return quad;
}
