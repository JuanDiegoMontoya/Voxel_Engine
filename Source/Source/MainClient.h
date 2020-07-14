#pragma once

namespace Client
{
	// runs in thread, updating in the background
	void Init();
	void MainLoop();
	void Shutdown();
	Net::ClientInput GetActions(); //
}