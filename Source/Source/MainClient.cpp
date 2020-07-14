#include "stdafx.h"
#include "MainClient.h"
#include <NetDefines.h>

#include "Renderer.h"
#include <Pipeline.h>
#include <camera.h>
#include "NetEvent.h"
#include <input.h>
#include <Timer.h>

#include <enet/enet.h>
#include <zlib.h>
#include <concurrent_queue.h>

#include <thread>
#include <sstream>

namespace Client
{
	Net::EventController eventQueue;
	std::unique_ptr<std::thread> thread;

	bool shutdownThreads = false;
	void Init()
	{
		thread = std::make_unique<std::thread>(MainLoop);
	}

#pragma optimize("", off)
	void MainLoop()
	{
		ENetAddress address;
		ENetHost* client;
		ENetPeer* peer;
		char message[1024];
		ENetEvent event;
		int eventStatus;

		// a. Initialize enet
		if (enet_initialize() != 0) {
			printf("An error occured while initializing ENet.\n");
			exit(EXIT_FAILURE);
		}

		atexit(enet_deinitialize);

		// b. Create a host using enet_host_create
		client = enet_host_create(NULL, 1, 2, 57600 / 8, 14400 / 8);

		if (client == NULL) {
			printf("An error occured while trying to create an ENet server host\n");
			exit(EXIT_FAILURE);
		}

		enet_address_set_host(&address, "localhost");
		address.port = 1234;

		// c. Connect and user service
		peer = enet_host_connect(client, &address, 2, 0);

		if (peer == NULL) {
			printf("No available peers for initializing an ENet connection");
			exit(EXIT_FAILURE);
		}

		eventStatus = 1;

		Timer timer;
		double accum = 0;
		while (!shutdownThreads)
		{
			accum += timer.elapsed();
			timer.reset();

			eventStatus = enet_host_service(client, &event, CLIENT_NET_TICK * 1000);

			// If we had some event that interested us
			if (eventStatus > 0) {
				switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					printf("(Client) We got a new connection from %x\n",
						event.peer->address.host);
					break;

				case ENET_EVENT_TYPE_RECEIVE:
					printf("(Client) Message from server : %s\n", event.packet->data);
					// Lets broadcast this message to all
					// enet_host_broadcast(client, 0, event.packet);
					enet_packet_destroy(event.packet);
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					printf("(Client) %s disconnected.\n", event.peer->data);

					// Reset client's information
					event.peer->data = NULL;
					break;
				}
			}

			while (accum > CLIENT_NET_TICK)
			{
				accum -= CLIENT_NET_TICK;

				auto p = Renderer::GetPipeline()->GetCamera(0)->GetPos();
				std::pair<int, Net::ClientPrintVec3Event> data;
				data.first = Net::Packet::eClientPrintVec3Event;
				data.second.clientID = 69;
				data.second.v = p;
				ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(peer, 0, packet);

				//char message[1000];
				//static int ttt = 0;
				//sprintf(message, "%d: (%f, %f, %f)\n", ttt++, p.x, p.y, p.z);
				//if (strlen(message) > 0)
				//{
				//	ENetPacket* packet = enet_packet_create(message, strlen(message) + 1, ENET_PACKET_FLAG_RELIABLE);
				//	enet_peer_send(peer, 0, packet);
				//	//enet_packet_destroy(packet);
				//}
			}
		}

		enet_peer_disconnect(peer, 0);
		/* Allow up to 3 seconds for the disconnect to succeed
		 * and drop any packets received packets.
		 */
		while (enet_host_service(client, &event, 3000) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_RECEIVE:
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("Disconnection succeeded.");
				return;
			}
		}
		/* We've arrived here, so the disconnect attempt didn't */
		/* succeed yet.  Force the connection down.             */
		enet_peer_reset(peer);
	}

	void Shutdown()
	{
		shutdownThreads = true;
		thread->join();
	}

	Net::ClientInput GetActions()
	{
		Net::ClientInput ret;
		ret.jump = Input::Keyboard().pressed[GLFW_KEY_SPACE];
		ret.moveBack = Input::Keyboard().down[GLFW_KEY_S];
		ret.moveFoward = Input::Keyboard().down[GLFW_KEY_W];
		ret.moveLeft = Input::Keyboard().down[GLFW_KEY_A];
		ret.moveRight = Input::Keyboard().down[GLFW_KEY_D];
	}

}