#pragma once
#include "BufferAllocator.h"
#include <Shapes.h>

namespace ChunkRenderer
{
	void InitAllocator();
	void GenerateDrawCommandsGPU();
	void GenerateDrawCommandsSplatGPU();
	void RenderNorm();
	void RenderSplat();

	void DrawBuffers();


	/* $$$$$$$$$$$$$$$   Culling pipeline stuff   $$$$$$$$$$$$$$$$

			phase 1:
				render normally
			phase 2:
				reset allocation data buffer
				generate DIB (frustum + distance cull)
			phase 3:
				occlusion draw bounding boxes (BBs)
					any chunk whose BB was drawn will be set to draw in next frame
					no color or depth is written in this phase
			phase 4 (maybe optional):
				render chunks whose visibility changed to true
				will lower FPS, but will prevent temporal occlusion holes

	$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

	void Render();                // phase 1
	void UpdateAllocDataBuffer(); // phase 2
	void GenerateDIB();           // phase 2
	void RenderOcclusion();       // phase 3
	void RenderRest();            // phase 4

	inline std::unique_ptr<BufferAllocator<AABB16>> allocator;
	inline std::unique_ptr<BufferAllocator<AABB16>> allocatorSplat;

	struct Settings
	{
		// visibility
		float normalMin = 0;
		float normalMax = 800;
		float splatMin = 800;
		float splatMax = 8000;
	}inline settings;
}