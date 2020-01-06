#include "stdafx.h"
#include "debug.h"

static bool initSysPerf = true;

namespace Debug
{
	std::map<std::string, double> systemsPerfInfo;


	void Update(float dt)
	{
		PERF_BENCHMARK_START;

		////function performance window
		//if (initSysPerf)
		//{
		//	ImVec2 Pos(1650.0f, 650.0f);
		//	ImVec2 Size(230.0f, 225.0f);
		//	ImGui::SetNextWindowPos(Pos);
		//	ImGui::SetNextWindowSize(Size);
		//	ImGui::SetNextWindowCollapsed(false);
		//}

		ImGui::Begin("Function Performance (ms)");

		//display values stored in systemsPerfInfo
		std::for_each(systemsPerfInfo.begin(), systemsPerfInfo.end(),
			[&](auto pair)
		{
			ImGui::Text("%-24s", pair.first.c_str());
			ImGui::SameLine();
			ImGui::PushItemWidth(-300);
			ImGui::Text("%.4f", pair.second);
		});

		ImGui::End();

		initSysPerf = false;

		systemsPerfInfo.clear();
		PERF_BENCHMARK_END; //this needs to be last to capture this function's perf
	}
}