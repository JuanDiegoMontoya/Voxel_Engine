#pragma once
#include <enet/enet.h>
#include <thread>
#include <memory>
#include "ClientInfo.h"

struct Packet;

namespace Net
{
	class Server
	{
	public:
		// launches thread that processes network events in the background
		bool Init();
		void Shutdown();

	private:
		void run();
		void cleanup();
		void ProcessClientEvent(Packet& packet);


		bool shutdownThreads = false;
		std::unique_ptr<std::thread> thread;

		ENetAddress address;
		ENetHost* server;
		ENetEvent event;
		int eventStatus;
	};
}