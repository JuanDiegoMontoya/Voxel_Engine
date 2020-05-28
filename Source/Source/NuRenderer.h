#pragma once

namespace NuRenderer
{
	void Init();
	void CompileShaders();
	void Clear();
	void DrawAll();

	void drawChunks();
	void splatChunks();
	void drawChunksWater();

	inline int drawCalls = 0;

	struct Settings
	{
		bool gammaCorrection = true;
	}inline settings;
}