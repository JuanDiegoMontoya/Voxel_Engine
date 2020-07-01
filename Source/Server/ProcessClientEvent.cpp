#include "stdafx.h"
#include "ProcessClientEvent.h"
#include <NetEvent.h>

namespace Net
{
		void ProcessClientEvent(Packet* packet)
		{
			auto& data = packet->data;
			switch (packet->type)
			{
				PROCESS_EVENT(PrintCameraPosEvent);
				PROCESS_EVENT(InputEvent);
			default: 
				//assert(false && "The type hasn't been defined yet!");
				printf("Unsupported data type sent from a client!");
			}
		}
}