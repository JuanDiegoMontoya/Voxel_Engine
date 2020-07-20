#include "stdafx.h"
#include "Client.h"
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

namespace Net
{
	void Client::Init()
	{
		thread = std::make_unique<std::thread>([this]() { MainLoop(); });
	}

#pragma optimize("", off)
	void Client::MainLoop()
	{
		// a. Initialize enet
		if (enet_initialize() != 0)
		{
			printf("An error occured while initializing ENet.\n");
			exit(EXIT_FAILURE);
		}

		atexit(enet_deinitialize);

		// b. Create a host using enet_host_create
		client = enet_host_create(NULL, 1, 2, 57600 / 8, 14400 / 8);

		if (client == NULL)
		{
			printf("An error occured while trying to create an ENet server host\n");
			exit(EXIT_FAILURE);
		}

		enet_address_set_host(&address, "localhost");
		address.port = 1234;

		// c. Connect and user service
		peer = enet_host_connect(client, &address, 2, 0);

		if (peer == NULL)
		{
			printf("No available peers for initializing an ENet connection");
			exit(EXIT_FAILURE);
		}

		// not sure why, but enet_host_service needs to be called before sending this packet
		enet_host_service(client, &event, CLIENT_NET_TICK * 1000);
		int join = Net::Packet::eClientJoinEvent;
		ENetPacket* joinRequest = enet_packet_create(&join, sizeof(join), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, joinRequest);

		Timer timer;
		double accum = 0;
		while (!shutdownThreads)
		{
			accum += timer.elapsed();
			timer.reset();

			eventStatus = enet_host_service(client, &event, CLIENT_NET_TICK * 1000);

			// If we had some event that interested us
			if (eventStatus > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:
				{
					printf("(Client) We got a new connection from %x\n",
						event.peer->address.host);
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					//printf("(Client) Message from server : %s\n", event.packet->data);
					// Lets broadcast this message to all
					// enet_host_broadcast(client, 0, event.packet);
					//enet_packet_destroy(event.packet);
					//printf("Received a message from the server (%x)\n", event.peer->address.host);

					Net::Packet packet(event.packet);
					printf("Received event from server of type: %d\n", packet.GetType());
					ProcessServerEvent(packet);
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					printf("(Client) %s disconnected.\n", event.peer->data);

					// Reset client's information
					event.peer->data = NULL;
					break;
				}
				}
			}

			while (accum > CLIENT_NET_TICK)
			{
				accum -= CLIENT_NET_TICK;

				// send client player state
				{
					auto temp = Renderer::GetPipeline()->GetCamera(0)->GetPos();
					Packet p(Packet::eClientPrintVec3Event, &temp, sizeof(ClientPrintVec3Event));
					ENetPacket* packet = enet_packet_create(p.GetBuffer(), sizeof(int) + sizeof(ClientPrintVec3Event), ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(peer, 0, packet);
				}

				// send client input
				{
					auto temp = GetActions();
					Packet p(Packet::eClientInput, &temp, sizeof(ClientInput));
					ENetPacket* packet = enet_packet_create(p.GetBuffer(), sizeof(int) + sizeof(ClientInput), ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(peer, 0, packet);
				}
			}
		}

		DisconnectFromCurrent();
	}

	void Client::Shutdown()
	{
		shutdownThreads = true;
		thread->join();
	}

	void Client::DisconnectFromCurrent()
	{
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

	Net::ClientInput Client::GetActions()
	{
		Net::ClientInput ret;
		ret.jump = Input::Keyboard().pressed[GLFW_KEY_SPACE];
		ret.moveBack = Input::Keyboard().down[GLFW_KEY_S];
		ret.moveFoward = Input::Keyboard().down[GLFW_KEY_W];
		ret.moveLeft = Input::Keyboard().down[GLFW_KEY_A];
		ret.moveRight = Input::Keyboard().down[GLFW_KEY_D];
		return ret;
	}

	void Client::ProcessServerEvent(Net::Packet& packet)
	{
		switch (packet.GetType())
		{
		case Packet::eServerJoinResultEvent:
		{
			processJoinResultEvent(packet);
			break;
		}
		case Packet::eServerListPlayersEvent:
		{
			processServerListPlayersEvent(packet);
			break;
		}
		default:
		{
			//assert(false && "The type hasn't been defined yet!");
			printf("Unsupported data type sent from the server! (%d)\n", packet.GetType());
		}
		}
	}

	void Client::processJoinResultEvent(Packet& packet)
	{
		auto event = *reinterpret_cast<ServerJoinResultEvent*>(packet.GetData());
		
		printf("Join result: %d\n", event.success);

		if (event.success)
		{
			printf("Our ID: %d\n", event.id);
			thisID = event.id;
		}
		else
			DisconnectFromCurrent();
	}

	void Client::processServerListPlayersEvent(Packet& packet)
	{
		auto event = *reinterpret_cast<ServerListPlayersEvent*>(packet.GetData());
		event.IDs = reinterpret_cast<int*>(packet.GetData() + sizeof(int));
		
		printf("we got a player list event! num players: %d\n", event.connected);

		for (int i = 0; i < event.connected; i++)
			printf("%d\n", event.IDs[i]);
	}
}