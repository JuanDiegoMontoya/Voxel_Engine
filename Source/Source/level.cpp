#include "stdafx.h"
//#include "block.h"
#include "camera.h"
#include "generation.h"
#include "level.h"
#include "pipeline.h"
#include "mesh.h"
#include "texture.h"
#include "frustum.h"

#include "transform.h"
#include "mesh_comp.h"
#include "unlit_mesh.h"
#include "render_data.h"
#include "chunk.h"
#include "sun.h"
//#include <omp.h>
#include <chrono>
#include <execution>
#include "vendor/ctpl_stl.h"
#include "input.h"
#include "pick.h"
#include "settings.h"
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
	cameras_.push_back(new Camera(kControlCam));
	Render::SetCamera(cameras_[0]);

	int cc = 4; // chunk count
	updatedChunks_.reserve(cc * cc * cc);

	high_resolution_clock::time_point benchmark_clock_ = high_resolution_clock::now();

	//WorldGen::GenerateSimpleWorld(cc, cc, cc, .999f, updatedChunks_);
	WorldGen::GenerateHeightMapWorld(cc, cc, this);

	// TODO: enable compiler C++ optimizations (currently disabled for debugging purposes)

	// TODO: call updateBlock() function AND add checking to update nearby chunks if necessary

	//std::cout << "PRE Processed chunk positions (x, y, z):" << '\n';
	//for (auto& chunk : Chunk::chunks)
	//{
	//	if (chunk.second)
	//	{
	//		const auto& poo = chunk.second->GetPos();
	//		std::cout << '(' << poo.x << ", " << poo.y << ", " << poo.z << ')' << std::endl;
	//	}
	//	else
	//	{
	//		const auto& poo = chunk.first;
	//		std::cout << "Disabled chunk at: " << '(' << poo.x << ", " << poo.y << ", " << poo.z << ')' << std::endl;
	//	}
	//}

	ProcessUpdatedChunks();
	TestCoordinateStuff();

	//std::cout << "POST Processed chunk positions (x, y, z):" << '\n';
	//for (auto& chunk : Chunk::chunks)
	//{
	//	if (chunk.second)
	//	{
	//		const auto& poo = chunk.second->GetPos();
	//		std::cout << '(' << poo.x << ", " << poo.y << ", " << poo.z << ')' << std::endl;
	//	}
	//	else
	//	{
	//		const auto& poo = chunk.first;
	//		std::cout << "Disabled chunk at: " << '(' << poo.x << ", " << poo.y << ", " << poo.z << ')' << std::endl;
	//	}
	//}
	
	duration<double> benchmark_duration_ = duration_cast<duration<double>>(high_resolution_clock::now() - benchmark_clock_);
	std::cout << benchmark_duration_.count() << std::endl;
}

// update every object in the level
void Level::Update(float dt)
{
	PERF_BENCHMARK_START;
	glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction

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

	std::for_each(
		std::execution::par_unseq,
		Chunk::chunks.begin(),
		Chunk::chunks.end(),
		[](auto& p)
	{
		if (p.second)
		{
			p.second->Update();
		}
	});

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

	glDisable(GL_FRAMEBUFFER_SRGB);
	PERF_BENCHMARK_END;
}

// unused
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
		if (chunk.second && chunk.second->IsVisible())
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
		if (chunk.second && chunk.second->IsVisible())// && sun_.GetFrustum()->IsInside(chunk.second) >= Frustum::Visibility::Partial)
		{
			currShader->setMat4("model", chunk.second->GetModel());
			chunk.second->Render();
		}
	});
	
	// 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static VAO* blockHoverVao = nullptr;
static VBO* blockHoverVbo = nullptr;
void Level::DrawDebug()
{
	{
		ImGui::SetNextWindowPos(ImVec2(20, 20));
		ImGui::SetNextWindowSize(ImVec2(400, 600));
		ImGui::Begin("Sun");

		glm::vec3 pos = sun_.GetPos();
		if (ImGui::DragFloat3("Sun Pos", &pos[0], 1, -500, 500, "%.0f"))
			sun_.SetPos(pos);

		glm::vec3 dir = sun_.GetDir();
		if (ImGui::SliderFloat3("Sun Dir", &dir[0], -1, 1, "%.3f"))
			sun_.SetDir(dir);

		ImGui::Checkbox("Orbit Pos", &sun_.orbits);
		ImGui::SameLine();
		ImGui::DragFloat3("##Orbitee", &sun_.orbitPos[0], 2.f, -500, 500, "%.0f");
		ImGui::Checkbox("Follow Cam", &sun_.followCam);
		ImGui::SliderFloat("Follow Distance", &sun_.followDist, 0, 500, "%.0f");

		float far = sun_.GetFarPlane();
		if (ImGui::SliderFloat("Far Plane", &far, 1, 1000, "%.0f"))
			sun_.SetFarPlane(far);

		ImGui::SliderFloat("Projection Window", &sun_.projSize, 0, 500, "%.0f");

		//int shadow = sun_.GetShadowSize().x;
		//if (ImGui::InputInt("Shadow Scale", &shadow, 1024, 1024))
		//{
		//	glm::clamp(shadow, 0, 16384);
		//	sun_.SetShadowSize(glm::ivec2(shadow));
		//}

		ImGui::End();
	}

	{
		ImGui::SetNextWindowPos(ImVec2(1500, 20));
		ImGui::SetNextWindowSize(ImVec2(400, 600));
		ImGui::Begin("Info");

		ImGui::Text("FPS: %.0f (%.1f ms)", 1.f / game_->GetDT(), 1000 * game_->GetDT());
		ImGui::NewLine();
		glm::vec3 pos = Render::GetCamera()->GetPos();
		//ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
		if (ImGui::InputFloat3("Camera Position", &pos[0], 2))
			Render::GetCamera()->SetPos(pos);
		pos = Render::GetCamera()->front;
		ImGui::Text("Camera Direction: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);

		localpos local = Chunk::worldBlockToLocalPos(Render::GetCamera()->GetPos());
		ImGui::Text("In chunk pos: (%d, %d, %d)", local.chunk_pos.x, local.chunk_pos.y, local.chunk_pos.z);
		ImGui::Text("In block pos: (%d, %d, %d)", local.block_pos.x, local.block_pos.y, local.block_pos.z);

		ImGui::NewLine();
		ImGui::Text("Chunk count: %d", Chunk::chunks.size());
		ImGui::Text("Chunk size: %d", Chunk::CHUNK_SIZE);
		int cnt = 0;
		for (auto& p : Chunk::chunks)
			if (p.second) cnt++;
		ImGui::Text("Non-null chunks: %d", cnt);

		ImGui::NewLine();
		ImGui::Text("Flying: %s", activeCursor ? "False" : "True");

		float dist = 5;
		ImGui::Text("Raycast information:");
		ImGui::Text("Ray length: %d", dist);
		raycast(
			Render::GetCamera()->GetPos(),
			Render::GetCamera()->front,
			dist,
			std::function<bool(float, float, float, BlockPtr, glm::vec3)>
			([&](float x, float y, float z, BlockPtr block, glm::vec3 side)->bool
		{
			if (!block || block->GetType() == Block::bAir)
				return false;

			ImGui::Text("Block pos:  (%.2f, %.2f, %.2f)", x, y, z);
			ImGui::Text("Block side: (%.2f, %.2f, %.2f)", side.x, side.y, side.z);
			//glm::vec3 color = Block::PropertiesTable[block->GetType()].color;
			//ImGui::ColorPicker3("colorr", )

			if (blockHoverVao == nullptr)
			{
				blockHoverVao = new VAO();
				blockHoverVbo = new VBO(Render::cube_vertices, sizeof(Render::cube_vertices));
				VBOlayout layout;
				layout.Push<float>(3);
				blockHoverVao->AddBuffer(*blockHoverVbo, layout);
			}

			glClear(GL_DEPTH_BUFFER_BIT);
			blockHoverVao->Bind();
			ShaderPtr curr = Shader::shaders["flat_color"];
			curr->Use();
			curr->setMat4("u_model", glm::translate(glm::mat4(1), glm::vec3(x, y, z) + .5f));
			curr->setMat4("u_view", Render::GetCamera()->GetView());
			curr->setMat4("u_proj", Render::GetCamera()->GetProj());
			curr->setVec4("u_color", glm::vec4(1, 1, 1, .2f));
			glDrawArrays(GL_TRIANGLES, 0, 36);

			return true;
		}
		));

		ImGui::End();
	}

	renderAxisIndicators();
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
			chunk->BuildMesh();
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

// handles everything that needs to be done when a block is changed
void Level::UpdateBlockAt(glm::ivec3 wpos, Block::BlockType ty)
{
	localpos p = Chunk::worldBlockToLocalPos(wpos);
	BlockPtr block = Chunk::AtWorld(wpos);
	ChunkPtr chunk = Chunk::chunks[p.chunk_pos];

	if (block && block->GetType() == ty) // ignore if same type
		return;
	
	// create empty chunk if it's null
	if (!chunk)
	{
		Chunk::chunks[p.chunk_pos] = chunk = new Chunk(true);
		chunk->SetPos(p.chunk_pos);
	}

	if (!block) // skip null blocks
		block = &chunk->At(p.block_pos);
	block->SetType(ty);

	// add to update list if it ain't
	if (!isChunkInUpdateList(chunk))
		updatedChunks_.push_back(chunk);

	// check if adjacent to opaque blocks in nearby chunks, then update those chunks if it is
	glm::ivec3 dirs[] =
	{
		{ -1,  0,  0 },
		{  1,  0,  0 },
		{  0, -1,  0 },
		{  0,  1,  0 },
		{  0,  0, -1 },
		{  0,  0,  1 }
	};
	for (const auto& dir : dirs)
	{
		checkUpdateChunkNearBlock(wpos, dir);
	}
}

bool Level::isChunkInUpdateList(ChunkPtr c)
{
	for (auto& chunk : updatedChunks_)
	{
		if (chunk == c)
			return true;
	}
	return false;
}

void Level::checkUpdateChunkNearBlock(const glm::ivec3& pos, const glm::ivec3& near)
{
	localpos p1 = Chunk::worldBlockToLocalPos(pos);
	localpos p2 = Chunk::worldBlockToLocalPos(pos + near);
	if (p1.chunk_pos == p2.chunk_pos)
		return;

	// update chunk if near block is NOT air/invisible
	BlockPtr cb = Chunk::AtWorld(pos);
	BlockPtr nb = Chunk::AtWorld(pos + near);
	if (cb && nb && nb->GetType() != Block::bAir)
		if (!isChunkInUpdateList(Chunk::chunks[p2.chunk_pos]))
			updatedChunks_.push_back(Chunk::chunks[p2.chunk_pos]);
}

void Level::checkBlockPlacement()
{
	if (Input::Mouse().pressed[GLFW_MOUSE_BUTTON_2])
	{
		raycast(
			Render::GetCamera()->GetPos(),
			Render::GetCamera()->front,
			5,
			std::function<bool(float, float, float, BlockPtr, glm::vec3)>
			([&](float x, float y, float z, BlockPtr block, glm::vec3 side)->bool
		{
			if (!block || block->GetType() == Block::bAir)
				return false;

			UpdateBlockAt(glm::ivec3(x, y, z) + glm::ivec3(side), Block::bStone);

			//Chunk::AtWorld(glm::ivec3(x, y, z) + glm::ivec3(side))->SetType(Block::bStone);
			//for (auto& chunk : updatedChunks_)
			//{
			//	if (chunk == Chunk::chunks[Chunk::worldBlockToLocalPos(glm::ivec3(x, y, z)).chunk_pos])
			//		return false;
			//}
			//updatedChunks_.push_back(Chunk::chunks[Chunk::worldBlockToLocalPos(glm::ivec3(x, y, z)).chunk_pos]);
			
			return true;
		}
		));
	}
}

void Level::checkBlockDestruction()
{
	if (Input::Mouse().pressed[GLFW_MOUSE_BUTTON_1] && 
		!ImGui::IsAnyItemHovered() && 
		!ImGui::IsAnyItemActive() && 
		!ImGui::IsAnyItemFocused())
	{
		raycast(
			Render::GetCamera()->GetPos(),
			Render::GetCamera()->front,
			5,
			std::function<bool(float, float, float, BlockPtr, glm::vec3)>
			([&](float x, float y, float z, BlockPtr block, glm::vec3 side)->bool
		{
			if (!block || block->GetType() == Block::bAir)
				return false;

			UpdateBlockAt(glm::ivec3(x, y, z), Block::bAir);

			//block->SetType(Block::bAir);
			//for (auto& chunk : updatedChunks_)
			//{
			//	if (chunk == Chunk::chunks[Chunk::worldBlockToLocalPos(glm::ivec3(x, y, z)).chunk_pos])
			//		return false;
			//}
			//updatedChunks_.push_back(Chunk::chunks[Chunk::worldBlockToLocalPos(glm::ivec3(x, y, z)).chunk_pos]);
			
			return true;
		}
		));
	}
}

static unsigned int axisIndicatorVAO;
static unsigned int axisIndicatorVBO;
static VAO* axisVAO;
static VBO* axisVBO;
void renderAxisIndicators()
{
	if (axisVAO == nullptr)
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

		axisVAO = new VAO();
		axisVBO = new VBO(indicatorVertices, sizeof(indicatorVertices), GL_STATIC_DRAW);
		VBOlayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		axisVAO->AddBuffer(*axisVBO, layout);
	}
	/* Renders the axis indicator (a screen-space object) as though it were
		one that exists in the world for simplicity. */
	ShaderPtr currShader = Shader::shaders["axis"];
	currShader->Use();
	Camera* cam = Render::GetCamera();
	currShader->setMat4("u_model", glm::translate(glm::mat4(1), cam->GetPos() + cam->front * 10.f)); // add scaling factor (larger # = smaller visual)
	currShader->setMat4("u_view", cam->GetView());
	currShader->setMat4("u_proj", cam->GetProj());
	glClear(GL_DEPTH_BUFFER_BIT); // allows indicator to always be rendered
	axisVAO->Bind();
	glLineWidth(2.f);
	glDrawArrays(GL_LINES, 0, 6);
	axisVAO->Unbind();
}

// draw the shadow map/view of the world from the sun's perspective
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