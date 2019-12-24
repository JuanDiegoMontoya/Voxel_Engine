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
#include <chrono>
#include <execution>
#include "vendor/ctpl_stl.h"
#include "input.h"
#include "pick.h"
#include "settings.h"
#include <functional>
#include "editor.h"

#include <set>
#include <memory>
#include "collision_shapes.h"

using namespace std::chrono;

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
	//cameras_.push_back(new Camera(kControlCam));
	cameras_.push_back(new Camera(kControlCam));
	Render::SetCamera(cameras_[0]);
	
	//config = std::make_shared<btDefaultCollisionConfiguration>();
	//dispatcher = std::make_shared<btCollisionDispatcher>(config.get());
	//btInterface = std::make_shared<btSimpleBroadphase>();


	high_resolution_clock::time_point benchmark_clock_ = high_resolution_clock::now();

	chunkManager_.Init();
	WorldGen::InitNoiseFuncs();
	Editor::level = this;
	Editor::chunkManager = &chunkManager_;
	Editor::renderer = &renderer_;
	PrefabManager::InitPrefabs();
	BiomeManager::InitializeBiomes();
	chunkManager_.SetCurrentLevel(this);
	chunkManager_.SetLoadDistance(100.f);
	chunkManager_.SetUnloadLeniency(100.f);
	chunkManager_.SetMaxLoadPerFrame(1);
	renderer_.Init();
	renderer_.chunkManager_ = &chunkManager_;
	
	duration<double> benchmark_duration_ = duration_cast<duration<double>>(high_resolution_clock::now() - benchmark_clock_);
	std::cout << benchmark_duration_.count() << std::endl;
}


// update every object in the level
void Level::Update(float dt)
{
	if (Input::Keyboard().pressed[GLFW_KEY_GRAVE_ACCENT])
	{
		activeCursor = !activeCursor;
	}
	glfwSetInputMode(game_->GetWindow(), GLFW_CURSOR, activeCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

	// update each camera
	if (!activeCursor)
		for (auto& cam : cameras_)
			cam->Update(dt);

	chunkManager_.Update(this);
	CheckInteraction();
	sun_.Update();
	renderer_.SetDirLight(&sun_.GetDirLight());
	renderer_.SetSun(&sun_);

	if (doCollisionTick)
		CheckCollision();

	renderer_.DrawAll();
	Editor::Update();
	hud_.Update();
	DrawImGui();
}


void Level::DrawImGui()
{
	{
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
		ImGui::Checkbox("Collision Enabled", &doCollisionTick);

		bool val = Render::GetCamera()->GetType() == kPhysicsCam;
		if (ImGui::Checkbox("Camera Gravity", &val))
		{
			Render::GetCamera()->SetType(val ? kPhysicsCam : kControlCam);
		}

		//int shadow = sun_.GetShadowSize().x;
		//if (ImGui::InputInt("Shadow Scale", &shadow, 1024, 1024))
		//{
		//	glm::clamp(shadow, 0, 16384);
		//	sun_.SetShadowSize(glm::ivec2(shadow));
		//}
		if (ImGui::Button("Recompile Water Shader"))
		{
			delete Shader::shaders["chunk_water"];
			Shader::shaders["chunk_water"] = new Shader("chunk_water.vs", "chunk_water.fs");
		}
		if (ImGui::Button("Recompile Debug Map"))
		{
			delete Shader::shaders["debug_map3"];
			Shader::shaders["debug_map3"] = new Shader("debug_map.vs", "debug_map.fs");
		}
		if (ImGui::Button("Recompile Postprocess Shader"))
		{
			//delete Shader::shaders["postprocess"];
			Shader::shaders["postprocess"] = new Shader("postprocess.vs", "postprocess.fs");
		}
		if (ImGui::Button("Delete far chunks (unsafe)"))
		{
			std::vector<ChunkPtr> deleteList;
			std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
				[&](auto& p)
			{
				float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Render::GetCamera()->GetPos());
				if (p.second && dist > chunkManager_.loadDistance_ + chunkManager_.unloadLeniency_)
				{
					deleteList.push_back(p.second);
					p.second = nullptr;
				}
			});

			for (ChunkPtr p : deleteList)
				delete p;
		}

		ImGui::End();
	}

	{
		ImGui::Begin("Info");

		ImGui::Text("FPS: %.0f (%.1f ms)", 1.f / game_->GetDT(), 1000 * game_->GetDT());
		ImGui::NewLine();
		glm::vec3 pos = Render::GetCamera()->GetPos();
		//ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
		ImGui::SliderFloat("Render distance", &chunkManager_.loadDistance_, 0, 1000, "%.0f");
		ImGui::SliderFloat("Leniency distance", &chunkManager_.unloadLeniency_, 0, 1000, "%.0f");
		float far = Render::GetCamera()->GetFar();
		if (ImGui::SliderFloat("Far plane", &far, 0, 1000, "%.0f"))
			Render::GetCamera()->SetFar(far);
		if (ImGui::InputFloat3("Camera Position", &pos[0], 2))
			Render::GetCamera()->SetPos(pos);
		pos = Render::GetCamera()->front;
		ImGui::Text("Camera Direction: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
		pos = pos * .5f + .5f;
		ImGui::SameLine();
		ImGui::ColorButton("visualization", ImVec4(pos.x, pos.y, pos.z, 1.f));

		localpos local = Chunk::worldBlockToLocalPos(Render::GetCamera()->GetPos());
		ImGui::Text("In chunk pos: (%d, %d, %d)", local.chunk_pos.x, local.chunk_pos.y, local.chunk_pos.z);
		ImGui::Text("In block pos: (%d, %d, %d)", local.block_pos.x, local.block_pos.y, local.block_pos.z);

		ImGui::NewLine();
		ImGui::Text("Chunk size: %d", Chunk::CHUNK_SIZE);
		int nonNull = 0;
		int active = 0;
		for (auto& p : Chunk::chunks)
		{
			if (p.second)
			{
				nonNull++;
				if (p.second->IsActive())
					active++;
			}
		}
		ImGui::Text("Total chunks:    %d", Chunk::chunks.size());
		ImGui::Text("Non-null chunks: %d", nonNull);
		ImGui::Text("Active chunks:   %d", active);

		ImGui::NewLine();
		// displaying zero just means the queue was taken, not finished!
		ImGui::Text("Gen queue:    %d", chunkManager_.generation_queue_.size());
		ImGui::Text("Mesh queue:   %-4d (%d)", chunkManager_.mesher_queue_.size(), chunkManager_.debug_cur_pool_left.load());
		ImGui::Text("Buffer queue: %d", chunkManager_.buffer_queue_.size());

		ImGui::NewLine();
		ImGui::Text("Flying: %s", activeCursor ? "False" : "True");

		static bool init = true;
		if (!init)
		{
			ImGui::NewLine();
			const glm::vec3 camPos = Render::GetCamera()->GetPos();
			float t = WorldGen::GetTemperature(camPos.x, camPos.y, camPos.z);
			float h = WorldGen::GetHumidity(camPos.x, camPos.z);
			WorldGen::TerrainType tt = WorldGen::GetTerrainType(camPos);
			ImGui::Text("Biome info: ");
			ImGui::Text("Temperature: %.2f", t);
			ImGui::Text("Humidity: %.2f", h);
			ImGui::Text("Terrain: %d", (unsigned)tt);
			ImGui::Text("Biome name: %s", BiomeManager::GetBiome(t, h, tt));
			ImGui::NewLine();
		}
		init = false;

		int dist = 5;
		ImGui::Text("Voxel raycast information:");
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

			ImGui::Text("Block Type: %d (%s)", (unsigned)block->GetType(), block->GetName());
			ImGui::Text("Write Strength: %d", block->WriteStrength());
			ImGui::Text("Light Value: %d", block->LightValue());
			LightPtr lit = Chunk::LightAtWorld({ x, y, z });
			ImGui::Text("Light: (%d, %d, %d, %d)", lit->GetR(), lit->GetG(), lit->GetB(), lit->GetS());
			ImGui::Text("Block pos:  (%.2f, %.2f, %.2f)", x, y, z);
			ImGui::Text("Block side: (%.2f, %.2f, %.2f)", side.x, side.y, side.z);
			//glm::vec3 color = Block::PropertiesTable[block->GetType()].color;
			//ImGui::ColorPicker3("colorr", )

			ShaderPtr curr = Shader::shaders["flat_color"];
			curr->Use();
			curr->setMat4("u_model", glm::translate(glm::mat4(1), glm::vec3(x, y, z) + .5f));
			curr->setMat4("u_view", Render::GetCamera()->GetView());
			curr->setMat4("u_proj", Render::GetCamera()->GetProj());
			curr->setVec4("u_color", glm::vec4(1, 1, 1, .4f));
			glLineWidth(2);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			renderer_.DrawCube();

			return true;
		}
		));

		ImGui::End();
	}

	// TODO: make toggling shadows/reflections ACTUALLY disable them completely
	{
		ImGui::Begin("Global Settings");
		if (ImGui::Checkbox("Compute baked AO", &Settings::GFX::blockAO))
			chunkManager_.ReloadAllChunks();
		if (ImGui::Checkbox("Shadows", &renderer_.renderShadows))
			renderer_.ClearCSM();
		if (ImGui::Checkbox("Reflections", &renderer_.doGeometryPass))
			renderer_.ClearGGuffer();
		ImGui::NewLine();
		ImGui::Text("Post processing:");
		ImGui::Checkbox("Sharpen Filter", &renderer_.ppSharpenFilter);
		ImGui::Checkbox("Blur Filter", &renderer_.ppBlurFilter);
		ImGui::Checkbox("Edge detection", &renderer_.ppEdgeDetection);
		ImGui::Checkbox("Chromatic Aberration", &renderer_.ppChromaticAberration);
		ImGui::End();
	}
}


// TODO: move this into own file for physics/collision or something, just don't have it here
void Level::CheckCollision()
{
	auto cam = Render::GetCamera();
	ImGui::Begin("Collision");
	Box camBox(*cam);
	auto min = glm::ivec3(glm::floor(camBox.min));
	auto max = glm::ivec3(glm::ceil(camBox.max));
	auto mapcomp = [&camBox](const Box& a, const Box& b)->bool
	{
		return glm::distance(camBox.GetPosition(), a.GetPosition()) < glm::distance(camBox.GetPosition(), b.GetPosition());
	};
	std::set<Box, decltype(mapcomp)> blocks(mapcomp);
	for (int x = min.x; x < max.x; x++)
	{
		for (int y = min.y; y < max.y; y++)
		{
			for (int z = min.z; z < max.z; z++)
			{
				Box block({ x, y, z });
				blocks.insert(block);
				ImGui::Text("Checking (%d, %d, %d)", x, y, z);
			}
		}
	}
	for (auto& blockBox : blocks)
	{
		auto pos = blockBox.blockpos;
		if (GetBlockAt(pos).GetType() != Block::bAir)
		{
			// the normal of each face.
			constexpr glm::vec3 faces[6] =
			{
				{-1, 0, 0 }, // 'left' face normal (-x direction)
				{ 1, 0, 0 }, // 'right' face normal (+x direction)
				{ 0, 1, 0 }, // 'bottom' face normal (-y direction)
				{ 0,-1, 0 }, // 'top' face normal (+y direction)
				{ 0, 0,-1 }, // 'far' face normal (-z direction)
				{ 0, 0, 1 }  // 'near' face normal (+x direction)
			};

			// distance of collided box to the face.
			float distances[6] =
			{
				(blockBox.max.x - camBox.min.x), // distance of box 'b' to face on 'left' side of 'a'.
				(camBox.max.x - blockBox.min.x), // distance of box 'b' to face on 'right' side of 'a'.
				(camBox.max.y - blockBox.min.y), // distance of box 'b' to face on 'bottom' side of 'a'.
				(blockBox.max.y - camBox.min.y), // distance of box 'b' to face on 'top' side of 'a'.
				(blockBox.max.z - camBox.min.z), // distance of box 'b' to face on 'far' side of 'a'.
				(camBox.max.z - blockBox.min.z), // distance of box 'b' to face on 'near' side of 'a'.
			};

			//https://www.gamedev.net/forums/topic/567310-platform-game-collision-detection/
			int collidedFace;
			float collisionDepth = std::numeric_limits<float>::max();
			glm::vec3 collisionNormal(0);
			// scan each face, make sure the box intersects,
			// and take the face with least amount of intersection
			// as the collided face.
			std::string sfaces[] = { "left", "right", "bottom", "top", "far", "near" };
			for (int i = 0; i < 6; i++)
			{
				//// box does not intersect face. So boxes don't intersect at all.
				//if (distances[i] < 0.0f)
				//	return false;

				// face of least intersection depth. That's our candidate.
				if ((i == 0) || (distances[i] < collisionDepth))
				{
					collidedFace = i;
					collisionNormal = faces[i];
					collisionDepth = distances[i];
				}
			}
			int normalComp = collisionNormal.x ? 0 : collisionNormal.y ? 1 : 2;
			//auto newPos = cam->oldPos;
			auto refl = glm::normalize(glm::reflect(glm::normalize(cam->GetPos() - cam->oldPos + .000001f), -collisionNormal));
			//refl[normalComp] /= 2;
			glm::vec3 newPos = cam->GetPos();
			if (GetBlockAt(pos).GetType() == Block::bWater)
			{
				cam->velocity_.y = 2;
			}
			else
			{
				newPos = cam->GetPos() + collisionDepth * -collisionNormal * 1.000f;
				ImGui::Text("%s", sfaces[collidedFace].c_str());
				ImGui::Text("Reflection: (%.2f, %.2f, %.2f)", refl.x, refl.y, refl.z);
				cam->velocity_[normalComp] = 0;
			}
			cam->SetPos(newPos);
			camBox = Box(*cam);
		}
	}
		//ImGui::NewLine();
	ImGui::End();
}


void Level::CheckInteraction()
{
	checkBlockPlacement();
	checkBlockDestruction();
}


// force updates a block in a location
void Level::UpdateBlockAt(glm::ivec3 wpos, Block bl)
{
	Block block = bl;
	block.SetWriteStrength(std::numeric_limits<unsigned char>::max() / 2);
	chunkManager_.UpdateBlock(wpos, block);
}


// updates a block in a location IFF the new block has a sufficiently high write strength
void Level::GenerateBlockAt(glm::ivec3 wpos, Block b)
{
	chunkManager_.UpdateBlock(wpos, b);
}


void Level::GenerateBlockAtCheap(glm::ivec3 wpos, Block b)
{
	chunkManager_.UpdateBlockCheap(wpos, b);
}


void Level::UpdatedChunk(ChunkPtr chunk)
{
	chunkManager_.UpdatedChunk(chunk);
}


Block Level::GetBlockAt(glm::ivec3 wpos)
{
	return chunkManager_.GetBlock(wpos);
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

			UpdateBlockAt(glm::ivec3(x, y, z) + glm::ivec3(side), hud_.selected_);

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
