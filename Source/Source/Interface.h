#pragma once

namespace Interface
{
	void Init();
	void Update();
	void DrawImGui();
	bool IsCursorActive();

	inline bool activeCursor = false;
	inline bool debug_graphs = true;
}