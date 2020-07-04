#include "stdafx.h"
#include "ProcessClientEvent.h"
#include <NetEvent.h>

namespace Net
{
		void ProcessClientEvent(Packet& packet)
		{
			auto& data = packet.data;
			if (data == nullptr)
			{
				printf("Recieved null data!\n");
				return;
			}
			switch (packet.type)
			{
			case Packet::eClientJoinEvent:
				printf("A client connected!\n");
				// TODO: map client's address to a unique ID, 
				// then send it back as a JoinResultEvent
				break;

			case Packet::eClientLeaveEvent:
				printf("A client disconnected!\n");
				break;

			case Packet::eClientPrintVec3Event:
				glm::vec3 v = reinterpret_cast<ClientPrintVec3Event*>(data)->v;
				printf("(%f, %f, %f)\n", v.x, v.y, v.z);
				break;

			case Packet::eClientInputEvent:
				int input = reinterpret_cast<ClientInputEvent*>(data)->input;
				printf("%d\n", input);
				// TODO: update client's physics state based on input
				break;

			default:
				//assert(false && "The type hasn't been defined yet!");
				printf("Unsupported data type sent from a client!\n");
			}
		}
}