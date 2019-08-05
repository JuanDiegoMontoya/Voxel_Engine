#include "stdafx.h"
#include "vbo.h"
#include "vao.h"
#include "ibo.h"
#include "chunk.h"
#include "block.h"
#include "camera.h"
#include "shader.h"
#include "pipeline.h"
#include "frustum.h"
#include <mutex>
#include <sstream>

/*
	TODO: use IBOs to save GPU memory
	TODO: ensure water shader is getting uniforms 'n' stuff
*/

//Concurrency::concurrent_unordered_map<glm::ivec3, Chunk*, Utils::ivec3Hash> Chunk::chunks;
std::unordered_map<glm::ivec3, Chunk*, Utils::ivec3Hash> Chunk::chunks;

double Chunk::isolevel = 0.9;

static std::mutex mtx;

Chunk::Chunk(bool active) : active_(active)
{
	vao_ = new VAO();
	wvao_ = new VAO();
}

Chunk::~Chunk()
{
	if (vao_)
		delete vao_;
	if (positions_)
		delete positions_;
	if (normals_)
		delete normals_;
	if (colors_)
		delete colors_;
	if (speculars_)
		delete speculars_;

	if (wvao_)
		delete wvao_;
	if (wpositions_)
		delete wpositions_;
	if (wnormals_)
		delete wnormals_;
	if (wcolors_)
		delete wcolors_;
	if (wspeculars_)
		delete wspeculars_;
}

void Chunk::Update()
{
	// cull chunks that are invisible
	if (Render::GetCamera()->GetFrustum()->IsInside(*this) >= Frustum::Visibility::Partial)
		visible_ = true;
	else
		visible_ = false;
	//visible_ = true;
	// build mesh after ensuring culled blocks are culled
}

void Chunk::Render()
{
	//ASSERT(vao_ && positions_ && normals_ && colors_);
	if (vao_)
	{
		vao_->Bind();
		glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
	}
}

void Chunk::RenderWater()
{
	if (wvao_)
	{
		wvao_->Bind();
		glDrawArrays(GL_TRIANGLES, 0, wvertexCount_);
	}
}

void Chunk::BuildBuffers()
{
	// generate various vertex buffers
	{
		vao_->Bind();
		if (positions_)
			delete positions_;
		if (normals_)
			delete normals_;
		if (colors_)
			delete colors_;
		if (speculars_)
			delete speculars_;
		// screen positions
		positions_ = new VBO(&tPositions[0], sizeof(glm::vec3) * tPositions.size());
		positions_->Bind();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0); // screenpos
		glEnableVertexAttribArray(0);

		// colors
		colors_ = new VBO(&tColors[0], sizeof(glm::vec4) * tColors.size());
		colors_->Bind();
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
		glEnableVertexAttribArray(1);

		// normals
		normals_ = new VBO(&tNormals[0], sizeof(glm::vec3) * tNormals.size());
		normals_->Bind();
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(2);

		// specular
		speculars_ = new VBO(&tSpeculars[0], sizeof(float) * tSpeculars.size());
		speculars_->Bind();
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
		glEnableVertexAttribArray(3);

		vao_->Unbind();
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		vertexCount_ = tPositions.size(); // divisor = number of floats per vertex
		tPositions.clear();
		tNormals.clear();
		tColors.clear();
		tSpeculars.clear();
	}

	// epic copypasta
	{
		wvao_->Bind();
		if (wpositions_)
			delete wpositions_;
		if (wnormals_)
			delete wnormals_;
		if (wcolors_)
			delete wcolors_;
		if (wspeculars_)
			delete wspeculars_;
		// screen positions
		wpositions_ = new VBO(&wtPositions[0], sizeof(glm::vec3) * wtPositions.size());
		wpositions_->Bind();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0); // screenpos
		glEnableVertexAttribArray(0);

		// colors
		wcolors_ = new VBO(&wtColors[0], sizeof(glm::vec4) * wtColors.size());
		wcolors_->Bind();
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
		glEnableVertexAttribArray(1);

		// normals
		wnormals_ = new VBO(&wtNormals[0], sizeof(glm::vec3) * wtNormals.size());
		wnormals_->Bind();
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(2);

		// specular
		wspeculars_ = new VBO(&wtSpeculars[0], sizeof(float) * wtSpeculars.size());
		wspeculars_->Bind();
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
		glEnableVertexAttribArray(3);

		wvao_->Unbind();
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		wvertexCount_ = wtPositions.size(); // divisor = number of floats per vertex
		wtPositions.clear();
		wtNormals.clear();
		wtColors.clear();
		wtSpeculars.clear();
	}
}

void Chunk::BuildMesh()
{
	//Camera* cam = Render::GetCamera();

	//vertices.reserve(CHUNK_SIZE * CHUNK_SIZE * 6 * 3); // one entire side of a chunk (assumed flat)
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				// skip fully transparent blocks
				if (At(x, y, z).GetType() == Block::bAir)
					continue;

				// check if each face would be obscured, and adds the ones that aren't to the vbo
				// obscured IF side is adjacent to opaque block
				// NOT obscured if side is adjacent to nothing or transparent block
				glm::ivec3 pos(x, y, z);
#if MARCHED_CUBES
				buildBlockVertices_marched_cubes(pos, At(x, y, z));
#else
				buildBlockVertices_normal(pos, Render::cube_norm_tex_vertices, 48, At(x, y, z));
#endif
			}
		}
	}
	// "buildbuffers" would normally happen here
	// 'vertices' is stored until "buildbuffers" is called
}

void Chunk::buildBlockVertices_normal(const glm::ivec3 & pos, const float * data, int quadStride, const Block& block)
{
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;

	// back
	buildSingleBlockFace(glm::vec3(x, y, z - 1), quadStride, 0, data, pos, block);

	// front
	buildSingleBlockFace(glm::ivec3(x, y, z + 1), quadStride, 1, data, pos, block);

	// left
	buildSingleBlockFace(glm::ivec3(x - 1, y, z), quadStride, 2, data, pos, block);

	// right
	buildSingleBlockFace(glm::ivec3(x + 1, y, z), quadStride, 3, data, pos, block);

	// bottom
	buildSingleBlockFace(glm::ivec3(x, y - 1, z), quadStride, 4, data, pos, block);

	// top
	buildSingleBlockFace(glm::ivec3(x, y + 1, z), quadStride, 5, data, pos, block);
}

//std::vector<float> Chunk::buildSingleBlockFace(glm::ivec3 near, int low, int high, int x, int y, int z)
void Chunk::buildSingleBlockFace(
	const glm::ivec3& nearFace,											// position of nearby block to check
	int quadStride, int curQuad, const float* data, // vertex + quad data
	const glm::ivec3& blockPos,											// position of current block
	const Block& block)															// block-specific information
{
	localpos nearblock = worldBlockToLocalPos(chunkBlockToWorldPos(nearFace));
	bool isWater = block.GetType() == Block::bWater;

	ChunkPtr near = chunks[nearblock.chunk_pos];
	if (!near)
		goto GenQuad;
	if (!near->active_)
		goto GenQuad;
	if (near->At(nearblock.block_pos).GetType() != Block::bAir && near->At(nearblock.block_pos).GetType() != Block::bWater)
		return;
	if (near->At(nearblock.block_pos).GetType() == Block::bWater && block.GetType() == Block::bWater)
		return;
	if (Block::PropertiesTable[block.GetType()].invisible)
		return;

GenQuad:
	// transform the vertices relative to the chunk
	// (the full world transformation will be completed in a shader)
	glm::mat4 localTransform = glm::translate(glm::mat4(1.f), glm::vec3(blockPos) + .5f); // non-scaled

	float shiny = Block::PropertiesTable[block.GetType()].specular;
	glm::vec4 color = Block::PropertiesTable[block.GetType()].color;

	// slightly randomize color for each block to make them more visible (temporary solution)
	float clrBias = Utils::get_random_r(-.03, .03);

	// sadly we gotta copy all this stuff 6 times
	for (int i = 0; i < 6; i++)
	{
		if (isWater)
		{
			wtColors.push_back(glm::vec4(glm::vec3(color.r, color.g, color.b), color.a));
			wtNormals.push_back(Render::cube_normals_divisor2[curQuad]);
			wtSpeculars.push_back(shiny);
		}
		else
		{
			tColors.push_back(glm::vec4(glm::vec3(color.r, color.g, color.b) + clrBias, color.a));
			tNormals.push_back(Render::cube_normals_divisor2[curQuad]);
			tSpeculars.push_back(shiny);
		}
	}

	for (int i = quadStride * curQuad; i < quadStride * (curQuad + 1); i += 8) // += floats per vertex
	{
		glm::vec4 tri(
			data[i + 0], 
			data[i + 1], 
			data[i + 2], 
			1.f);
		tri = localTransform * tri;

		// pos
		if (isWater)
			wtPositions.push_back(tri);
		else
			tPositions.push_back(tri);

		// texture
		//quad.push_back(data[i + 6]);
		//quad.push_back(data[i + 7]);
	}
}

static std::ostream& operator<<(std::ostream& o, glm::ivec3 v)
{
	return o << '('
		<< v.x << ", "
		<< v.y << ", "
		<< v.z << ')';
}

// prints to console if errors are found
void TestCoordinateStuff()
{
	using namespace std;
	for (auto& p : Chunk::chunks)
	{
		using namespace std;
		//cout << p.first << '\n';
		if (!p.second)
			continue;
		if (p.first != p.second->GetPos())
			cout << "Hash key " << p.first 
			<< " does not match chunk internal position" 
			<< p.second->GetPos() << '\n';

		for (int x = 0; x < Chunk::CHUNK_SIZE; x++)
		{
			for (int y = 0; y < Chunk::CHUNK_SIZE; y++)
			{
				for (int z = 0; z < Chunk::CHUNK_SIZE; z++)
				{
					glm::ivec3 posLocal(x, y, z);
					glm::ivec3 posActual = p.second->chunkBlockToWorldPos(posLocal);
					localpos posCalc = Chunk::worldBlockToLocalPos(posActual);

					if (posCalc.chunk_pos != p.first)
						cout << "Calculated chunk position " << 
						posCalc.chunk_pos << 
						" does not match hash key " << p.first;
					if (posCalc.block_pos != posLocal)
						cout << "Calculated block position " <<
						posCalc.block_pos <<
						" does not match local pos " << posLocal;
				}
			}
		}
	}
}