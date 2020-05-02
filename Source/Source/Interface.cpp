#include "stdafx.h"
#include "Interface.h"

#include <shader.h>
#include "World.h"
#include "Renderer.h"
#include <Pipeline.h>
#include <camera.h>
#include <Engine.h>
#include "pick.h"
#include "settings.h"
#include "ImGuiBonus.h"
#include <input.h>

namespace Interface
{
	void Init()
	{
		Engine::PushRenderCallback(DrawImGui, 1);
		Engine::PushRenderCallback(Update, 2);
	}

	void Update()
	{
		if (Input::Keyboard().pressed[GLFW_KEY_GRAVE_ACCENT])
			Interface::activeCursor = !Interface::activeCursor;
		glfwSetInputMode(Engine::GetWindow(), GLFW_CURSOR, Interface::activeCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}

	void DrawImGui()
	{
		{
			ImGui::Begin("Sun");

			glm::vec3 pos = World::sun_->GetPos();
			if (ImGui::DragFloat3("Sun Pos", &pos[0], 1, -500, 500, "%.0f"))
				World::sun_->SetPos(pos);

			glm::vec3 dir = World::sun_->GetDir();
			if (ImGui::SliderFloat3("Sun Dir", &dir[0], -1, 1, "%.3f"))
				World::sun_->SetDir(dir);

			ImGui::Checkbox("Orbit Pos", &World::sun_->orbits);
			ImGui::SameLine();
			ImGui::DragFloat3("##Orbitee", &World::sun_->orbitPos[0], 2.f, -500, 500, "%.0f");
			ImGui::Checkbox("Follow Cam", &World::sun_->followCam);
			ImGui::SliderFloat("Follow Distance", &World::sun_->followDist, 0, 500, "%.0f");
			ImGui::Checkbox("Collision Enabled", &World::doCollisionTick);

			bool val = Renderer::GetPipeline()->GetCamera(0)->GetType() == CameraType::kPhysicsCam;
			if (ImGui::Checkbox("Camera Has Gravity", &val))
			{
				Renderer::GetPipeline()->GetCamera(0)->SetType(val ? CameraType::kPhysicsCam : CameraType::kControlCam);
			}

			//int shadow = World::sun_->GetShadowSize().x;
			//if (ImGui::InputInt("Shadow Scale", &shadow, 1024, 1024))
			//{
			//	glm::clamp(shadow, 0, 16384);
			//	World::sun_->SetShadowSize(glm::ivec2(shadow));
			//}
			//if (ImGui::Button("Recompile Water Shader"))
			//{
			//	delete Shader::shaders["chunk_water"];
			//	Shader::shaders["chunk_water"] = new Shader("chunk_water.vs", "chunk_water.fs");
			//}
			//if (ImGui::Button("Recompile Debug Map"))
			//{
			//	delete Shader::shaders["debug_map3"];
			//	Shader::shaders["debug_map3"] = new Shader("debug_map.vs", "debug_map.fs");
			//}
			//if (ImGui::Button("Recompile Postprocess Shader"))
			//{
			//	//delete Shader::shaders["postprocess"];
			//	Shader::shaders["postprocess"] = new Shader("postprocess.vs", "postprocess.fs");
			//}
			if (ImGui::Button("Recompile Optimized Chunk Shader"))
			{
				delete Shader::shaders["chunk_optimized"];
				Shader::shaders["chunk_optimized"] = new Shader("chunk_optimized.vs", "chunk_optimized.fs");
			}

			if (ImGui::Button("Delete far chunks (unsafe)"))
			{
				std::vector<ChunkPtr> deleteList;
				std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
					[&](auto& p)
				{
					float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Renderer::GetPipeline()->GetCamera(0)->GetPos());
					if (p.second && dist > World::chunkManager_.loadDistance_ + World::chunkManager_.unloadLeniency_)
					{
						deleteList.push_back(p.second);
						p.second = nullptr;
					}
				});

				for (ChunkPtr p : deleteList)
					delete p;
			}

			static char fileName[256];
			ImGui::InputText("Map path", fileName, 256u);
			if (ImGui::Button("Save Map"))
			{
				World::chunkManager_.SaveWorld(fileName);
			}
			if (ImGui::Button("Load Map"))
			{
				World::chunkManager_.LoadWorld(fileName);
			}

			ImGui::End();
		}

		{

			ImGui::Begin("Info");
			ImGui::Text("FPS: %.0f (%.1f ms)", 1.f / Engine::GetDT(), Engine::GetDT() * 1000);

			ImGui::NewLine();
			glm::vec3 pos = Renderer::GetPipeline()->GetCamera(0)->GetPos();
			//ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
			ImGui::SliderFloat("Render distance", &World::chunkManager_.loadDistance_, 0, 1000, "%.0f");
			ImGui::SliderFloat("Leniency distance", &World::chunkManager_.unloadLeniency_, 0, 1000, "%.0f");
			float far = Renderer::GetPipeline()->GetCamera(0)->GetFar();
			if (ImGui::SliderFloat("Far plane", &far, 0, 1000, "%.0f"))
				Renderer::GetPipeline()->GetCamera(0)->SetFar(far);
			if (ImGui::InputFloat3("Camera Position", &pos[0], 2))
				Renderer::GetPipeline()->GetCamera(0)->SetPos(pos);
			pos = Renderer::GetPipeline()->GetCamera(0)->front;
			ImGui::Text("Camera Direction: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
			pos = pos * .5f + .5f;
			ImGui::SameLine();
			ImGui::ColorButton("visualization", ImVec4(pos.x, pos.y, pos.z, 1.f));

			localpos local = Chunk::worldBlockToLocalPos(Renderer::GetPipeline()->GetCamera(0)->GetPos());
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
					//active++;
				}
			}
			ImGui::Text("Total chunks:    %d", Chunk::chunks.size());
			ImGui::Text("Non-null chunks: %d", nonNull);
			ImGui::Text("Active chunks:   %d", active);

			ImGui::NewLine();
			// displaying zero just means the queue was taken, not finished!
			ImGui::Text("Gen queue:    %d", World::chunkManager_.generation_queue_.size());
			ImGui::Text("Mesh queue:   %-4d (%d)", World::chunkManager_.mesher_queue_.size(), World::chunkManager_.debug_cur_pool_left.load());
			ImGui::Text("Buffer queue: %d", World::chunkManager_.buffer_queue_.size());

			ImGui::NewLine();
			ImGui::Text("Flying: %s", activeCursor ? "False" : "True");

			static bool init = true;
			if (!init)
			{
				ImGui::NewLine();
				const glm::vec3 camPos = Renderer::GetPipeline()->GetCamera(0)->GetPos();
				float t = float(WorldGen::GetTemperature(camPos.x, camPos.y, camPos.z));
				float h = float(WorldGen::GetHumidity(camPos.x, camPos.z));
				TerrainType tt = WorldGen::GetTerrainType(camPos);
				ImGui::Text("Biome info: ");
				ImGui::Text("Temperature: %.2f", t);
				ImGui::Text("Humidity: %.2f", h);
				ImGui::Text("Terrain: %d", (unsigned)tt);
				ImGui::Text("Biome name: %s", BiomeManager::GetBiome(t, h, tt));
				ImGui::NewLine();
			}
			init = false;

			float dist = 5.f;
			ImGui::Text("Voxel raycast information:");
			ImGui::Text("Ray length: %0.f", dist);
			raycast(
				Renderer::GetPipeline()->GetCamera(0)->GetPos(),
				Renderer::GetPipeline()->GetCamera(0)->front,
				dist,
				std::function<bool(glm::vec3, BlockPtr, glm::vec3)>
				([&](glm::vec3 pos, BlockPtr block, glm::vec3 side)->bool
			{
				if (!block || block->GetType() == BlockType::bAir)
					return false;

				ImGui::Text("Block Type: %d (%s)", (unsigned)block->GetType(), block->GetName());
				ImGui::Text("Write Strength: %d", block->WriteStrength());
				ImGui::Text("Light Value: %d", block->LightValue());
				LightPtr lit = Chunk::LightAtWorld(pos);
				LightPtr lit2 = Chunk::LightAtWorld(pos + side);
				ImGui::Text("Light: (%d, %d, %d, %d)", lit->GetR(), lit->GetG(), lit->GetB(), lit->GetS());
				ImGui::Text("FLight: (%d, %d, %d, %d)", lit2->GetR(), lit2->GetG(), lit2->GetB(), lit2->GetS());
				ImGui::Text("Block pos:  (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
				ImGui::Text("Block side: (%.2f, %.2f, %.2f)", side.x, side.y, side.z);
				//glm::vec3 color = Block::PropertiesTable[block->GetType()].color;
				//ImGui::ColorPicker3("colorr", )

				ShaderPtr curr = Shader::shaders["flat_color"];
				curr->Use();
				curr->setMat4("u_model", glm::translate(glm::mat4(1), pos + .5f));
				curr->setMat4("u_view", Renderer::GetPipeline()->GetCamera(0)->GetView());
				curr->setMat4("u_proj", Renderer::GetPipeline()->GetCamera(0)->GetProj());
				curr->setVec4("u_color", glm::vec4(1, 1, 1, .4f));
				glLineWidth(2);
				GLint polygonMode;
				glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				Renderer::DrawCube();
				glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

				return true;
			}
			));

			ImGui::End();
		}

		// TODO: make toggling shadows/reflections ACTUALLY disable them completely
		{
			ImGui::Begin("Global Settings");
			if (ImGui::Checkbox("Compute baked AO", &Settings::GFX::blockAO))
				World::chunkManager_.ReloadAllChunks();
			if (ImGui::Checkbox("Skip lighting", &Chunk::debug_ignore_light_level))
				World::chunkManager_.ReloadAllChunks();
			if (ImGui::Checkbox("Shadows", &Renderer::renderShadows))
				Renderer::ClearCSM();
			if (ImGui::Checkbox("Reflections", &Renderer::doGeometryPass))
				Renderer::ClearGGuffer();
			ImGui::NewLine();
			ImGui::Text("Post processing:");
			ImGui::Checkbox("Sharpen Filter", &Renderer::ppSharpenFilter);
			ImGui::Checkbox("Blur Filter", &Renderer::ppBlurFilter);
			ImGui::Checkbox("Edge detection", &Renderer::ppEdgeDetection);
			ImGui::Checkbox("Chromatic Aberration", &Renderer::ppChromaticAberration);
			ImGui::End();
		}

		if (debug_graphs)
		{
			ImGui::Begin("Graphs");
			ImGui::Text("Avg Mesh Time: %.3f", Chunk::accumtime / Chunk::accumcount);
			ImGui::PlotVar("Frametime", Engine::GetDT(), FLT_MAX, FLT_MAX, 300, ImVec2(300, 100));

			if (Renderer::nvUsageEnabled)
			{
				GLint totalMemoryKb = 0;
				glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemoryKb);

				GLint currentMemoryKb = 0;
				glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentMemoryKb);
				ImGui::PlotVar("VRAM usage", (totalMemoryKb - currentMemoryKb) / 1000.f, 0, totalMemoryKb / 1000, 300, ImVec2(300, 100));
			}
			else
				ImGui::Text("VRAM usage graph disabled due to incompatible GPU");
			ImGui::End();
		}

		{
			ImGui::Begin("Debug");
			ImGui::Checkbox("Display Graphs", &debug_graphs);
			ImGui::End();
		}
	}
}