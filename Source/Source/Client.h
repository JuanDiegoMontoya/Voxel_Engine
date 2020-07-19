#pragma once

#include <NetEvent.h>
#include <enet/enet.h>
#include "NetPlayerState.h"

struct Packet;

namespace Net
{
	class Client
	{
	public:
		// runs in thread, updating in the background
		void Init();
		void Shutdown();
		void DisconnectFromCurrent();
	private:
		void MainLoop();

		Net::ClientInput GetActions();
		void ProcessServerEvent(Packet& packet);

		void processJoinResultEvent(Packet& packet);
		void processServerListPlayersEvent(Packet& packet);

		Net::EventController eventQueue;
		std::unique_ptr<std::thread> thread;

		bool shutdownThreads = false;

		ENetAddress address;
		ENetHost* client;
		ENetPeer* peer;
		ENetEvent event;
		int eventStatus;

		PlayerWorld playerWorld;
		int thisID;
	};
}