#include "stdafx.h"
#include <functional>
#include "input.h"
#include "block.h"
#include "camera.h"
#include "pipeline.h"
#include "pick.h"
#include "editor.h"
#include "shader.h"
#include "render.h"

namespace Editor
{
	// TODO: work on dis
	namespace RegionSelect
	{
		const int pickLength = 20;// ray cast distance
		size_t selectedPositions;	// how many positions have been selected
		glm::vec3 wpositions[3];	// selected positions (0-3)
		glm::vec3 hposition;			// hovered position (others are locked)

		void CancelSelection()
		{
			selectedPositions = 0;
		}

		void SelectBlock()
		{
			if (Input::Keyboard().pressed[GLFW_KEY_F])
			{
				ASSERT(selectedPositions >= 0 && selectedPositions <= 3);
				wpositions[selectedPositions] = hposition;
				if (selectedPositions < 3)
					selectedPositions++;
			}
		}

		void DrawSelection()
		{
			{
				ImGui::Begin("Selection Zone");
				int flag = (selectedPositions == 3 ? 0 : ImGuiButtonFlags_Disabled);
				if (ImGui::ButtonEx("save", ImVec2(0, 0), flag))
				{
					// save the prefab in a file or something fam
				}
				ImGui::Text("Selected positions: %d", selectedPositions);
				ImGui::Text("Hovered   : (%.2f, %.2f, %.2f)", hposition.x, hposition.y, hposition.z);
				ImGui::Text("Position 0: (%.2f, %.2f, %.2f)", wpositions[0].x, wpositions[0].y, wpositions[0].z);
				ImGui::Text("Position 1: (%.2f, %.2f, %.2f)", wpositions[1].x, wpositions[1].y, wpositions[1].z);
				ImGui::Text("Position 2: (%.2f, %.2f, %.2f)", wpositions[2].x, wpositions[2].y, wpositions[2].z);
				ImGui::End();
			}

			// actually draw the bounding box
			glm::vec3 pos(0);
			glm::vec3 scale(0);
			if (selectedPositions == 0)
			{
				scale = glm::vec3(1);
				pos = hposition;
			}
			else if (selectedPositions == 1)
			{
				// component wise
				glm::vec3 min(
					glm::min(wpositions[0].x, hposition.x),
					glm::min(wpositions[0].y, hposition.y),
					glm::min(wpositions[0].z, hposition.z));
				glm::vec3 max(
					glm::max(wpositions[0].x, hposition.x),
					glm::max(wpositions[0].y, hposition.y),
					glm::max(wpositions[0].z, hposition.z));

				pos = (wpositions[0] + hposition) / 2.f;
				scale = glm::abs(max - min);
			}
			else if (selectedPositions == 2)
			{
				// component wise
				glm::vec3 min(
					glm::min(wpositions[0].x, glm::min(wpositions[1].x, hposition.x)),
					glm::min(wpositions[0].y, glm::min(wpositions[1].y, hposition.y)),
					glm::min(wpositions[0].z, glm::min(wpositions[1].z, hposition.z)));
				glm::vec3 max(
					glm::max(wpositions[0].x, glm::max(wpositions[1].x, hposition.x)),
					glm::max(wpositions[0].y, glm::max(wpositions[1].y, hposition.y)),
					glm::max(wpositions[0].z, glm::max(wpositions[1].z, hposition.z)));

				pos = (wpositions[0] + wpositions[1] + hposition) / 3.f;
				scale = glm::abs(max - min);
			}
			else// if (selectedPositions == 3)
			{
				// component wise
				glm::vec3 min(
					glm::min(wpositions[0].x, glm::min(wpositions[1].x, wpositions[2].x)),
					glm::min(wpositions[0].y, glm::min(wpositions[1].y, wpositions[2].y)),
					glm::min(wpositions[0].z, glm::min(wpositions[1].z, wpositions[2].z)));
				glm::vec3 max(
					glm::max(wpositions[0].x, glm::max(wpositions[1].x, wpositions[2].x)),
					glm::max(wpositions[0].y, glm::max(wpositions[1].y, wpositions[2].y)),
					glm::max(wpositions[0].z, glm::max(wpositions[1].z, wpositions[2].z)));

				pos = (wpositions[0] + wpositions[1] + wpositions[2]) / 3.f;
				scale = glm::abs(max - min);
			}

			glm::mat4 tPos = glm::translate(glm::mat4(1), pos + .5f);
			glm::mat4 tScale = glm::scale(glm::mat4(1), scale);

			glDisable(GL_CULL_FACE);
			ShaderPtr curr = Shader::shaders["flat_color"];
			curr->Use();
			curr->setMat4("u_model", tPos * tScale);
			curr->setMat4("u_view", Render::GetCamera()->GetView());
			curr->setMat4("u_proj", Render::GetCamera()->GetProj());
			curr->setVec4("u_color", glm::vec4(1.f, .3f, 1.f, .4f));
			renderer->DrawCube();
			glEnable(GL_CULL_FACE);
		}

		void Update()
		{
			if (Input::Keyboard().down[GLFW_KEY_TAB])
			{
				raycast(
					Render::GetCamera()->GetPos(),
					Render::GetCamera()->front,
					20,
					[&](float x, float y, float z, BlockPtr block, glm::vec3 side)->bool
				{
					if (!block || block->GetType() == Block::bAir)
						return false;
					if (selectedPositions == 1 || selectedPositions == 3 || selectedPositions == 0)
						hposition = glm::vec3(x, y, z);
					else // if  (selectedPositions == 2)
					{
						// choose 2 axes that produce largest difference between first block, then use original value for the third
						glm::vec3 diff = glm::vec3(x, y, z) - wpositions[0];
						float smol = std::min(diff.x, std::min(diff.y, diff.z));
						if (smol == diff.x)
							hposition = glm::vec3(wpositions[0].x, y, z);
						else if (smol == diff.y)
							hposition = glm::vec3(x, wpositions[0].y, z);
						else
							hposition = glm::vec3(x, y, wpositions[0].z);
					}
					SelectBlock();
					return true;
				});

				DrawSelection();
			}
			else
			{
				CancelSelection();
			}
		}
	}

	void Update()
	{
		RegionSelect::Update();
	}
}