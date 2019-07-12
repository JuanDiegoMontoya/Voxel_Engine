#include "stdafx.h"
#include "level.h"
#include "pipeline.h"
#include "mesh.h"
#include "texture.h"

#include "transform.h"
#include "mesh_comp.h"
#include "unlit_mesh.h"
#include "render_data.h"
#include "block.h"
#include "chunk.h"
#include "sun.h"
//#include <omp.h>
#include <chrono>
#include <execution>
#include "vendor/ctpl_stl.h"
#include "input.h"
#include "pick.h"
#include <functional>

using namespace std::chrono;
//#define OMP_NUM_THREADS = 8;

Level::Level(std::string name)
{
	name_ = name;
}

Level::~Level()
{
	for (auto& obj : objects_)
		delete obj;

	for (auto& cam : cameras_)
		delete cam;
}

// for now this function is where we declare objects
void Level::Init()
{
	std::memset(Block::blocksarr_, 0, sizeof(float) * 100 * 100 * 100);

	cameras_.push_back(new Camera(kControlCam));
	Render::SetCamera(cameras_[0]);

	int cc = 1; // chunk count
	updatedChunks_.reserve(cc * cc * cc);
	sizeof(Block);
	// initialize a single chunk

	high_resolution_clock::time_point benchmark_clock_ = high_resolution_clock::now();

	// this loop is not parallelizable (unless VAO constructor is moved)
	for (int xc = 0; xc < cc * 2; xc++)
	{
		for (int yc = 0; yc < cc; yc++)
		{
			for (int zc = 0; zc < cc * 2; zc++)
			{
				Chunk* init = Chunk::chunks[glm::ivec3(xc, yc, zc)] = new Chunk(true);
				init->SetPos(glm::ivec3(xc, yc, zc));
				updatedChunks_.push_back(init);

				for (int x = 0; x < Chunk::CHUNK_SIZE; x++)
				{
					for (int y = 0; y < Chunk::CHUNK_SIZE; y++)
					{
						for (int z = 0; z < Chunk::CHUNK_SIZE; z++)
						{
							if (Utils::get_random_r(0, 1) > 1.99f)
								continue;
							init->At(x, y, z).SetType(Block::bStone);
						}
					}
				}
			}
		}
	}

	// TODO: enable compiler optimizations
	// TODO: figure out why chunk positions are (maybe) screwed up epically

/*
#define YEET

#ifdef YEET
	int chk = 3;
	for (int i = 0; i < chk; i++)
	{
#endif
#pragma omp parallel for num_threads(8)
#ifdef YEET
		for (int xc = cc / chk * i; xc < cc; xc++)
		{
			if (xc >= cc / chk * (i + 1))
				break;
#else
for (int xc = 0; xc < cc; xc++)
{
#endif
			for (int yc = 0; yc < cc; yc++)
			{
				for (int zc = 0; zc < cc; zc++)
				{
					Chunk::chunks[glm::ivec3(xc, yc, zc)]->Update();
				}
			}
		}

		// this loop cannot be parallelized
#ifdef YEET
		for (int xc = cc / chk * i; xc < cc / chk * (i + 1) && xc < cc; xc++)
#else
		for (int xc = 0; xc < cc; xc++)
#endif
		{
			for (int yc = 0; yc < cc; yc++)
			{
				for (int zc = 0; zc < cc; zc++)
				{
					Chunk::chunks[glm::ivec3(xc, yc, zc)]->BuildBuffers();
				}
			}
		}
#ifdef YEET
	}
#endif
*/

	ProcessUpdatedChunks();
	
	duration<double> benchmark_duration_ = duration_cast<duration<double>>(high_resolution_clock::now() - benchmark_clock_);
	std::cout << benchmark_duration_.count() << std::endl;
}

// update every object in the level
void Level::Update(float dt)
{
	ProcessUpdatedChunks();
	CheckInteraction();

	if (Input::Keyboard().pressed[GLFW_KEY_GRAVE_ACCENT])
	{
		activeCursor = !activeCursor;
	}
	glfwSetInputMode(game_->GetWindow(), GLFW_CURSOR, activeCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

	// update each camera
	if (!activeCursor)
		for (auto& cam : cameras_)
			cam->Update(dt);

	sun_.Update();
	DrawShadows(); // write to shadow map
	sun_.Render(); // render sun as the first "actual" object
	DrawNormal();
	DrawDebug();

	// debug shadows
	if (Input::Keyboard().down[GLFW_KEY_4])
	{
		Shader::shaders["debug_shadow"]->Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sun_.GetDepthTex());
		renderQuad();
	}
}

void Level::Draw()
{
	//glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // don't forget to reset original culling face
	glViewport(0, 0, 1920, 1080);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// render blocks in each active chunk
	std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
		[&](std::pair<glm::ivec3, Chunk*> chunk)
	{
		if (chunk.second)
			chunk.second->Render();
	});
}

void Level::DrawNormal()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // don't forget to reset original culling face

	// render blocks in each active chunk
	ShaderPtr currShader = Shader::shaders["chunk_shaded"];
	currShader->Use();
	currShader->setMat4("u_view", Render::GetCamera()->GetView());
	currShader->setMat4("u_proj", Render::GetCamera()->GetProj());
	currShader->setVec3("viewPos", Render::GetCamera()->GetPos());
	currShader->setVec3("lightPos", sun_.GetPos());
	currShader->setMat4("lightSpaceMatrix", sun_.GetViewProj());

	currShader->setVec3("dirLight.direction", sun_.GetDir());
	currShader->setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
	currShader->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	currShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sun_.GetDepthTex());
	std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
		[&](std::pair<glm::ivec3, Chunk*> chunk)
	{
		if (chunk.second)
		{
			currShader->setMat4("u_model", chunk.second->GetModel());
			chunk.second->Render();
		}
	});
}

void Level::DrawShadows()
{
	// 1. render depth of scene to texture (from light's perspective)
	//glDisable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);

	ShaderPtr currShader = Shader::shaders["shadow"];
	currShader->Use();
	currShader->setMat4("lightSpaceMatrix", sun_.GetViewProj());

	glViewport(0, 0, sun_.GetShadowSize().x, sun_.GetShadowSize().y);
	glBindFramebuffer(GL_FRAMEBUFFER, sun_.GetDepthFBO());
	glClear(GL_DEPTH_BUFFER_BIT);

	// render blocks in each active chunk
	std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
		[&](std::pair<glm::ivec3, Chunk*> chunk)
	{
		if (chunk.second)
		{
			currShader->setMat4("model", chunk.second->GetModel());
			chunk.second->Render();
		}
	});

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 1920, 1080);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Level::DrawDebug()
{
	ImGui::SetNextWindowPos(ImVec2(20, 20));
	ImGui::SetNextWindowSize(ImVec2(400, 600));
	ImGui::Begin("Sun");

	glm::vec3 pos = sun_.GetPos();
	if (ImGui::DragFloat3("Sun Pos", &pos[0], 1, -500, 500, "%.0f"))
		sun_.SetPos(pos);

	glm::vec3 dir = sun_.GetDir();
	if (ImGui::DragFloat3("Sun Dir", &dir[0], .01f, -1, 1, "%.3f"))
		sun_.SetDir(dir);

	ImGui::Checkbox("Orbit Pos", &sun_.orbits);
	ImGui::SameLine();
	ImGui::DragFloat3("##Orbitee", &sun_.orbitPos[0], 1, -500, 500, "%.0f");
	ImGui::Checkbox("Follow Cam", &sun_.followCam);
	ImGui::DragFloat("Follow Distance", &sun_.followDist, .5, 0, 500, "%.0f");

	float far = sun_.GetFarPlane();
	if (ImGui::DragFloat("Far Plane", &far, 2, 1, 1000, "%.0f"))
		sun_.SetFarPlane(far);

	ImGui::SliderFloat("Projection Window", &sun_.projSize, 0, 500, "%.0f");

	//int shadow = sun_.GetShadowSize().x;
	//if (ImGui::InputInt("Shadow Scale", &shadow, 1024, 1024))
	//{
	//	glm::clamp(shadow, 0, 16384);
	//	sun_.SetShadowSize(glm::ivec2(shadow));
	//}

	ImGui::End();

	//glViewport(1920 / 2, 1080 / 2, 50, 50); // X by Y pixel square at the center
	//glm::mat4 model = glm::rotate
	Shader::shaders["axis"]->Use();
	renderAxisIndicators();
	//glViewport(0, 0, 1920, 1080);
}

void Level::CheckCollision()
{
}

void Level::CheckInteraction()
{
	checkBlockPlacement();
	checkBlockDestruction();
}

void Level::ProcessUpdatedChunks()
{
	std::for_each(
		std::execution::par_unseq,
		updatedChunks_.begin(),
		updatedChunks_.end(),
		[](ChunkPtr& chunk)
	{
		if (chunk)
			chunk->Update();
	});

	// this operation cannot be parallelized
	std::for_each(
		std::execution::seq,
		updatedChunks_.begin(),
		updatedChunks_.end(),
		[](ChunkPtr& chunk)
	{
		if (chunk)
			chunk->BuildBuffers();
	});

	updatedChunks_.clear();
}

void Level::checkBlockPlacement()
{
	if (Input::Mouse().pressed[GLFW_MOUSE_BUTTON_2])
	{

	}
}

void Level::checkBlockDestruction()
{
	if (Input::Mouse().pressed[GLFW_MOUSE_BUTTON_1])
	{
		raycast(
			Render::GetCamera()->GetPos(),
			Render::GetCamera()->front,
			10,
			std::function<bool(float, float, float, BlockPtr, glm::vec3)>
			([&](float x, float y, float z, BlockPtr block, glm::vec3 side)->bool
		{
			if (!block || block->GetType() == Block::bAir)
				return false;
			//Chunk::AtWorld(glm::ivec3(x, y, z))->SetType(Block::bStone);
			block->SetType(Block::bStone);
			updatedChunks_.push_back(Chunk::chunks[Chunk::worldBlockToLocalPos(glm::ivec3(x, y, z)).chunk_pos]);
			return true;
		}
		));
	}
}

static unsigned int axisIndicatorVAO;
static unsigned int axisIndicatorVBO;
void renderAxisIndicators()
{
	if (axisIndicatorVAO == 0)
	{
		float indicatorVertices[] =
		{
			// positions			// colors
			0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // x-axis
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // y-axis
			0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // z-axis
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
		};

		glGenVertexArrays(1, &axisIndicatorVAO);
		glGenBuffers(1, &axisIndicatorVBO);
		glBindVertexArray(axisIndicatorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, axisIndicatorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(indicatorVertices), &indicatorVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}
	glBindVertexArray(axisIndicatorVAO);
	//glDrawArrays(GL_LINES, 0, 6);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

static unsigned int quadVAO = 0;
static unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] =
		{
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}