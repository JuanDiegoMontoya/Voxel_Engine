#include "stdafx.h"
#include <functional>
#include "input.h"
#include "block.h"
#include "camera.h"
#include "pipeline.h"
#include "pick.h"
#include "editor.h"

namespace Editor
{

	namespace RegionSelect
	{
		const int pickLength = 20;
		size_t selectedPositions;
		glm::vec3 wpositions[3];

		void CancelSelection()
		{
			selectedPositions = 0;
		}

		void Update()
		{
			if (Input::Keyboard().down[GLFW_KEY_LEFT_SHIFT])
			{
						raycast(
							Render::GetCamera()->GetPos(),
							Render::GetCamera()->front,
							20,
							[&](float x, float y, float z, BlockPtr block, glm::vec3 side)->bool
						{
							if (!block || block->GetType() == Block::bAir)
								return false;


							if (Input::Keyboard().pressed[GLFW_KEY_F])
							{
								selectedPositions++;
								switch (selectedPositions)
								{
								case 1:
								{

									break;
								}
								case 2:
								{

									break;
								}
								case 3:
								{

									break;
								}
								default:
									break;
								}

								return true;
							}
							else
							{
								CancelSelection();
							}
						});
				}
		}

		void SelectBlock()
		{

		}

		void DrawSelection()
		{
			
		}
	}

	void Update()
	{
		RegionSelect::Update();
	}
}