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
#include <sstream>
#include "settings.h"
#include "misc_utils.h"

/*
	TODO: use IBOs to save GPU memory
*/

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
		if (Render::GetCamera()->GetFrustum()->IsInside(*this) >= Frustum::Visibility::Partial)
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
		glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
	}
}


void Chunk::RenderWater()
{
	if (active_ && wvao_)
	{
		wvao_->Bind();
		glDrawArrays(GL_TRIANGLES, 0, wvertexCount_);
	}
}


void Chunk::BuildBuffers()
{
	std::lock_guard<std::mutex> lock(vertex_buffer_mutex_);
	// generate various vertex buffers
	{
		if (!vao_)
			vao_ = new VAO;
		if (!wvao_)
			wvao_ = new VAO;
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


#include <iomanip>
std::atomic<double> accumtime = 0;
std::atomic<unsigned> accumcount = 0;
void Chunk::BuildMesh()
{
	high_resolution_clock::time_point benchmark_clock_ = high_resolution_clock::now();

	std::lock_guard<std::mutex> lock(vertex_buffer_mutex_);
	for (int z = 0; z < CHUNK_SIZE; z++)
	{
		// precompute
		int zcsq = z * CHUNK_SIZE_SQRED;
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			// precompute
			int yczcsq = y * CHUNK_SIZE + zcsq;
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				//int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQRED;
				int index = x + yczcsq;

				// skip fully transparent blocks
				const Block block = blocks[index];
				if (Block::PropertiesTable[block.GetTypei()].invisible)
					continue;

				glm::ivec3 pos(x, y, z);
#if MARCHED_CUBES
				//buildBlockVertices_marched_cubes(pos, At(x, y, z));
				buildBlockVertices_marched_cubes(pos, block);
#else
				//buildBlockVertices_normal(pos, Render::cube_norm_tex_vertices, 48, At(x, y, z));
				buildBlockVertices_normal(pos, Render::cube_norm_tex_vertices, 48, block);
#endif
			}
		}
	}

	duration<double> benchmark_duration_ = duration_cast<duration<double>>(high_resolution_clock::now() - benchmark_clock_);
	double milliseconds = benchmark_duration_.count() * 1000;
	accumtime.store(accumtime + milliseconds);
	accumcount.store(accumcount + 1);
	std::cout 
		<< std::setw(-2) << std::showpoint << std::setprecision(4) << accumtime / accumcount << " ms "
		<< "(" << milliseconds << ")"
		<< std::endl;
}


void Chunk::buildBlockVertices_normal(const glm::ivec3 & pos, const float * data, int quadStride, const Block& block)
{
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;

	// back
	buildSingleBlockFace(glm::ivec3(x, y, z - 1), quadStride, 0, data, pos, block);

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
	const Block& block,															// block-specific information
	bool force)																			// force building of this block, if it exists
{
	localpos nearblock = worldBlockToLocalPos(chunkBlockToWorldPos(nearFace));
	bool isWater = block.GetType() == BlockType::bWater;
	ChunkPtr near = this;
	if (this->pos_ != nearblock.chunk_pos)
	{
		near = chunks[nearblock.chunk_pos];
		ASSERT(this != near);
	}
	Block block2; // near block
	Light light2; // near light
	if (force)
		goto GenQuad;
	if (!near)
		goto GenQuad;
	if (!near->active_)
		goto GenQuad;


	{
		block2 = near->At(nearblock.block_pos);
		light2 = near->LightAt(nearblock.block_pos);
		if (block2.GetType() != BlockType::bWater && block.GetType() == BlockType::bWater && (nearFace - blockPos).y > 0)
			goto GenQuad;
		if (block2.GetType() != BlockType::bAir && block2.GetType() != BlockType::bWater)
			return;
		if (block2.GetType() == BlockType::bWater && block.GetType() == BlockType::bWater)
			return;
		if (Block::PropertiesTable[int(block.GetType())].invisible)
			return;
	}

GenQuad:
	// transform the vertices relative to the chunk
	// (the full world transformation will be completed in a shader)
	glm::mat4 localTransform = glm::translate(glm::mat4(1.f), glm::vec3(blockPos) + .5f); // non-scaled

	float shiny = Block::PropertiesTable[int(block.GetType())].specular;
	glm::vec4 color = Block::PropertiesTable[int(block.GetType())].color;

	if (!debug_ignore_light_level)
	{
		color.r *= (1 + float(light2.GetR())) / 16.f;
		color.g *= (1 + float(light2.GetG())) / 16.f;
		color.b *= (1 + float(light2.GetB())) / 16.f;
	}

	// slightly randomize color for each block to make them more visible (temporary solution)
	//float clrBias = Utils::get_random_r(-.03f, .03f);

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
			tNormals.push_back(Render::cube_normals_divisor2[curQuad]);
			tSpeculars.push_back(shiny);
		}
	}

	// loop should iterate 6 times
	for (int i = quadStride * curQuad; i < quadStride * (curQuad + 1); i += 8) // += floats per vertex
	{
		glm::vec4 vert(
			data[i + 0], 
			data[i + 1], 
			data[i + 2], 
			1.f);
		glm::vec4 finalvert = localTransform * vert;

		// pos
		if (isWater)
			wtPositions.push_back(finalvert);
		else
		{
			float invOcclusion = 1;
			if (Settings::Graphics.blockAO)
				invOcclusion = computeBlockAO(block, blockPos, glm::vec3(vert), nearFace);
			tColors.push_back(glm::vec4((glm::vec3(color.r, color.g, color.b)) * invOcclusion, color.a));
			tPositions.push_back(finalvert);
		}

		// texture
		//quad.push_back(data[i + 6]);
		//quad.push_back(data[i + 7]);
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