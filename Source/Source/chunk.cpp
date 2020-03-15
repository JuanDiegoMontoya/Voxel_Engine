#include "stdafx.h"

#include <Pipeline.h>
#include "Renderer.h"
#include <camera.h>
#include <Frustum.h>

#include "vbo.h"
#include "vao.h"
#include "ibo.h"
#include "chunk.h"
#include "block.h"
#include "shader.h"
#include <Vertices.h>
#include <sstream>
#include "settings.h"
#include "misc_utils.h"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SIMD_AVX2

//Concurrency::concurrent_unordered_map<glm::ivec3, Chunk*, Utils::ivec3Hash> Chunk::chunks;
//std::unordered_map<glm::ivec3, Chunk*, Utils::ivec3Hash> Chunk::chunks;

double Chunk::isolevel = 0.60;

Chunk::Chunk(bool active) : active_(active)
{
	std::fill(std::begin(blocks), std::end(blocks), Block(BlockType::bAir));
	std::memset(lightMap, 0, sizeof(lightMap));
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


Chunk::Chunk(const Chunk& other)
{
	*this = other;
}


// copy assignment operator for serialization
Chunk& Chunk::operator=(const Chunk& rhs)
{
	//this->pos_ = rhs.pos_;
	this->SetPos(rhs.pos_);
	std::copy(std::begin(rhs.blocks), std::end(rhs.blocks), std::begin(this->blocks));
	std::copy(std::begin(rhs.lightMap), std::end(rhs.lightMap), std::begin(this->lightMap));
	return *this;
}


void Chunk::Update()
{
	// cull chunks that are invisible
	if (active_)
	{
		if (Renderer::GetPipeline()->GetCamera(0)->GetFrustum()->IsInside(bounds) >= Frustum::Visibility::Partial)
			visible_ = true;
		else
			visible_ = false;
	}

	// in the future, make this function perform other tick update actions,
	// such as updating N random blocks (like in Minecraft)
}


void Chunk::Render()
{
	//ASSERT(vao_ && positions_ && normals_ && colors_);
	if (active_ && vao_)
	{
		vao_->Bind();
		//glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
		ibo_->Bind();
		glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, (void*)0);
	}
}


void Chunk::RenderWater()
{
	if (active_ && wvao_)
	{
		wvao_->Bind();
		//glDrawArrays(GL_TRIANGLES, 0, wvertexCount_);
		wibo_->Bind();
		glDrawElements(GL_TRIANGLES, windexCount_, GL_UNSIGNED_INT, (void*)0);
	}
}


void Chunk::BuildBuffers()
{
	std::lock_guard<std::mutex> lock(vertex_buffer_mutex_);
	// generate various vertex buffers
	{
		if (!vao_)
			vao_ = new VAO;
		vao_->Bind();
		if (positions_)
			delete positions_;
		if (normals_)
			delete normals_;
		if (colors_)
			delete colors_;
		if (speculars_)
			delete speculars_;
		if (ibo_)
			delete ibo_;
		if (sunlight_)
			delete sunlight_;

		ibo_ = new IBO(&tIndices[0], tIndices.size());

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

		// sunlight
		sunlight_ = new VBO(&tSunlight[0], sizeof(float) * tSunlight.size());
		sunlight_->Bind();
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
		glEnableVertexAttribArray(4);

		vao_->Unbind();
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// delete mesh from CPU since it's no longer needed here
		vertexCount_ = tPositions.size();
		indexCount_ = tIndices.size();
		tPositions.clear();
		tNormals.clear();
		tColors.clear();
		tSpeculars.clear();
		tIndices.clear();
		tSunlight.clear();
	}

	// epic copypasta
	{
		if (!wvao_)
			wvao_ = new VAO;
		wvao_->Bind();
		if (wpositions_)
			delete wpositions_;
		if (wnormals_)
			delete wnormals_;
		if (wcolors_)
			delete wcolors_;
		if (wspeculars_)
			delete wspeculars_;
		if (wibo_)
			delete wibo_;
		if (wsunlight_)
			delete wsunlight_;

		wibo_ = new IBO(&wtIndices[0], wtIndices.size());

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

		wsunlight_ = new VBO(&wtSunlight[0], sizeof(float) * wtSunlight.size());
		wsunlight_->Bind();
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
		glEnableVertexAttribArray(4);

		wvao_->Unbind();
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		wvertexCount_ = wtPositions.size();
		windexCount_ = wtIndices.size();
		wtPositions.clear();
		wtNormals.clear();
		wtColors.clear();
		wtSpeculars.clear();
		wtIndices.clear();
		wtSunlight.clear();
	}
}


#include <iomanip>
void Chunk::BuildMesh()
{
	high_resolution_clock::time_point benchmark_clock_ = high_resolution_clock::now();

	std::lock_guard<std::mutex> lock(vertex_buffer_mutex_);
	for (int z = 0; z < CHUNK_SIZE; z++)
	{
		// precompute first flat index part
		int zcsq = z * CHUNK_SIZE_SQRED;
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			// precompute second flat index part
			int yczcsq = y * CHUNK_SIZE + zcsq;
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				// this is what we would be doing every innermost iteration
				//int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQRED;
				// we only need to do addition
				int index = x + yczcsq;

				// skip fully transparent blocks
				const Block block = blocks[index];
				if (Block::PropertiesTable[block.GetTypei()].invisible)
					continue;

#if MARCHED_CUBES
				//buildBlockVertices_marched_cubes(pos, At(x, y, z));
				buildBlockVertices_marched_cubes({ x, y, z }, block);
#else
				//buildBlockVertices_normal(pos, Render::cube_norm_tex_vertices, 48, At(x, y, z));
				buildBlockVertices_normal({ x, y, z }, block);
#endif
			}
		}
	}

	duration<double> benchmark_duration_ = duration_cast<duration<double>>(high_resolution_clock::now() - benchmark_clock_);
	double milliseconds = benchmark_duration_.count() * 1000;
	if (accumcount > 1000)
	{
		accumcount = 0;
		accumtime = 0;
	}
	accumtime = accumtime + milliseconds;
	accumcount = accumcount + 1;
	//std::cout 
	//	<< std::setw(-2) << std::showpoint << std::setprecision(4) << accumtime / accumcount << " ms "
	//	<< "(" << milliseconds << ")"
	//	<< std::endl;
}


void Chunk::buildBlockVertices_normal(const glm::ivec3& pos, Block block)
{
	for (int f = Far; f < fCount; f++)
		buildBlockFace(f, pos, block);
}


void Chunk::buildBlockFace(
	int face,
	const glm::ivec3& blockPos,	// position of current block
	Block block)								// block-specific information)
{
	thread_local static localpos nearblock; // avoids unnecessary construction of vec3s
	glm::ivec3 nearFace = blockPos + faces[face];

	//localpos nearblock = worldBlockToLocalPos(chunkBlockToWorldPos(nearFace));
	fastWorldBlockToLocalPos(chunkBlockToWorldPos(nearFace), nearblock);
	ChunkPtr nearChunk = this;
	if (this->pos_ != nearblock.chunk_pos)
	{
		nearChunk = chunks[nearblock.chunk_pos];
		ASSERT(this != nearChunk);
	}

	// for now, we won't make a mesh for faces adjacent to NULL chunks \
	in the future it may be wise to construct the mesh
	if (!nearChunk || !nearChunk->active_)
		return;

	// nearby block
	Block block2 = nearChunk->BlockAtCheap(nearblock.block_pos);
	Light light = nearChunk->LightAtCheap(nearblock.block_pos);
	if (block2.GetType() != BlockType::bWater && block.GetType() == BlockType::bWater && (nearFace - blockPos).y > 0)
	{
		addQuad(blockPos, block, face, nearChunk, light);
		return;
	}
	if (block2.GetType() != BlockType::bAir && block2.GetType() != BlockType::bWater)
		return;
	if (block2.GetType() == BlockType::bWater && block.GetType() == BlockType::bWater)
		return;
	if (Block::PropertiesTable[int(block.GetType())].invisible)
		return;

	// if all tests are passed, generate this face of the block
	addQuad(blockPos, block, face, nearChunk, light);
}


void Chunk::addQuad(const glm::ivec3& lpos, Block block, int face, ChunkPtr nearChunk, Light light)
{
	bool isWater = block.GetType() == BlockType::bWater;

	float shiny = Block::PropertiesTable[int(block.GetType())].specular;
	glm::vec4 color = Block::PropertiesTable[int(block.GetType())].color;

	if (!debug_ignore_light_level)
	{
		color.r *= (1 + float(light.GetR())) / 16.f;
		color.g *= (1 + float(light.GetG())) / 16.f;
		color.b *= (1 + float(light.GetB())) / 16.f;
	}

	// slightly randomize color for each block to make them more visible (temporary solution)
	//float clrBias = Utils::get_random_r(-.03f, .03f);

	// TODO: make sure this isn't always 1 when sunlight is properly added
	//float sun = float(light.GetS()) / 15.f;
	float sun = 1;

	// sadly we gotta copy all this stuff 4 times
	for (int i = 0; i < 4; i++)
	{
		if (isWater)
		{
			wtColors.push_back(glm::vec4(glm::vec3(color.r, color.g, color.b), color.a));
			wtNormals.push_back(faces[face]);
			wtSpeculars.push_back(shiny);
			wtSunlight.push_back(sun);
		}
		else
		{
			tNormals.push_back(faces[face]);
			tSpeculars.push_back(shiny);
			tSunlight.push_back(sun);
		}
	}

	// loop should iterate 6 times
	//for (int i = quadStride * curQuad; i < quadStride * (curQuad + 1); i += 8) // += floats per vertex
	//{
	//	glm::vec4 vert(
	//		data[i + 0],
	//		data[i + 1],
	//		data[i + 2],
	//		1.f);
	//	glm::vec4 finalvert = localTransform * vert;
	//	if (isWater)
	//		wtPositions.push_back(finalvert);
	//	else
	//	{
	//		// TODO: call "compute block AO" once per block or vertex way before this
	//		// because shared vertices means this is called too often
	//		float invOcclusion = 1;
	//		if (Settings::Graphics.blockAO)
	//			invOcclusion = computeBlockAO(block, blockPos, glm::vec3(vert), nearFace);
	//		tColors.push_back(glm::vec4((glm::vec3(color.r, color.g, color.b)) * invOcclusion, color.a));
	//		tPositions.push_back(finalvert);
	//	}
	//}

	// add 4 vertices representing a quad
	const GLfloat* data = Vertices::cube_light;
	int endQuad = (face + 1) * 12;
	for (int i = face * 12; i < endQuad; i += 3)
	{
		glm::vec3 vert(
			data[i + 0],
			data[i + 1],
			data[i + 2]
		);
		// transform vertices relative to chunk
		glm::vec3 finalVert = vert + glm::vec3(lpos) + .5f;

		if (!isWater)
		{
			float invOcclusion = 1;
			if (Settings::Graphics.blockAO)
				invOcclusion = computeBlockAO(block, lpos, glm::vec3(vert), lpos + faces[face]);
			tColors.push_back(glm::vec4((glm::vec3(color.r, color.g, color.b)) * invOcclusion, color.a));
			tPositions.push_back(finalVert);
		}
		else
			wtPositions.push_back(finalVert);
	}

	// add 6 indices defining 2 triangles from that quad
	int endIndices = (face + 1) * 6;
	for (int i = face * 6; i < endIndices; i++)
	{
		// refer to just placed vertices (4 of them)
		if (!isWater)
			tIndices.push_back(Vertices::cube_indices_light_cw[i] + tPositions.size() - 4);
		else
			wtIndices.push_back(Vertices::cube_indices_light_cw[i] + wtPositions.size() - 4);
	}
}


// computes how many of the two adjacent faces to the corner (that aren't this block) are solid
// returns a brightness scalar
float Chunk::computeBlockAO(
	Block block,
	const glm::ivec3& blockPos,
	const glm::vec3& corner,
	const glm::ivec3& nearFace)
{
	glm::ivec3 normal = nearFace - blockPos;
	glm::ivec3 combined = glm::ivec3(corner * 2.f) + normal; // convert corner to -1 to 1 range
	glm::ivec3 c1(0);
	glm::ivec3 c2(0);
	for (int i = 0; i < 3; i++)
	{
		if (glm::abs(combined[i]) != 2)
		{
			if (combined[i] && c1 == glm::ivec3(0))
				c1[i] = combined[i];
			else if (c2 == glm::ivec3(0))
				c2[i] = combined[i];
		}
	}
	
	float occlusion = 1;
	const float occAmt = .2f;

	// block 1
	localpos nearblock1 = worldBlockToLocalPos(chunkBlockToWorldPos(blockPos + c1 + normal));
	auto findVal1 = chunks.find(nearblock1.chunk_pos);
	ChunkPtr near1 = findVal1 == chunks.end() ? nullptr : findVal1->second;
	if (!near1)
		goto B2AO;
	if (!near1->active_)
		goto B2AO;
	if (near1->At(nearblock1.block_pos).GetType() != BlockType::bAir && near1->At(nearblock1.block_pos).GetType() != BlockType::bWater)
		occlusion -= occAmt;

B2AO:
	// block 2
	localpos nearblock2 = worldBlockToLocalPos(chunkBlockToWorldPos(blockPos + c2 + normal));
	auto findVal2 = chunks.find(nearblock2.chunk_pos);
	ChunkPtr near2 = findVal2 == chunks.end() ? nullptr : findVal2->second;
	if (!near2)
		goto BAO_END;
	if (!near2->active_)
		goto BAO_END;
	if (near2->At(nearblock2.block_pos).GetType() != BlockType::bAir && near2->At(nearblock2.block_pos).GetType() != BlockType::bWater)
		occlusion -= occAmt;

	// check diagonal if not 'fully' occluded
	if (occlusion > 1 - 2.f * occAmt)
	{
		// block 3
		localpos nearblock3 = worldBlockToLocalPos(chunkBlockToWorldPos(blockPos + c1 + c2 + normal));
		auto findVal3 = chunks.find(nearblock3.chunk_pos);
		ChunkPtr near3 = findVal3 == chunks.end() ? nullptr : findVal3->second;
		if (!near3)
			goto BAO_END;
		if (!near3->active_)
			goto BAO_END;
		if (near3->At(nearblock3.block_pos).GetType() != BlockType::bAir && near3->At(nearblock3.block_pos).GetType() != BlockType::bWater)
			occlusion -= occAmt;
	}

BAO_END:
	return occlusion;
}


// TODO: when making this function, use similar near chunk checking scheme to
// minimize amount of searching in the global chunk map
float Chunk::vertexFaceAO(const glm::vec3& corner, const glm::ivec3& faceNorm)
{
	Block side1, side2, corn;

	return 0.0f;
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