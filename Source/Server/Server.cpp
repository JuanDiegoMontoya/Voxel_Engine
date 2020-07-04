#include "stdafx.h"
#include <iostream>
#include <enet/enet.h>

#include <zlib.h>
#include <cassert>

#include <NetEvent.h>
#include "ProcessClientEvent.h"

void testCompress()
{
	std::string compressThis = { R"100(
Now, this is a story all about how
My life got flipped-turned upside down
And I'd like to take a minute
Just sit right there
I'll tell you how I became the prince of a town called Bel Air
In west Philadelphia born and raised
On the playground was where I spent most of my days
Chillin' out maxin' relaxin' all cool
And all shootin some b-ball outside of the school
When a couple of guys who were up to no good
Started making trouble in my neighborhood
I got in one little fight and my mom got scared
She said 'You're movin' with your auntie and uncle in Bel Air'
I begged and pleaded with her day after day
But she packed my suit case and sent me on my way
She gave me a kiss and then she gave me my ticket.
I put my Walkman on and said, 'I might as well kick it'.
First class, yo this is bad
Drinking orange juice out of a champagne glass.
Is this what the people of Bel-Air living like?
Hmm this might be alright.
But wait I hear they're prissy, bourgeois, all that
Is this the type of place that they just send this cool cat?
I don't think so
I'll see when I get there
I hope they're prepared for the prince of Bel-Air
Well, the plane landed and when I came out
There was a dude who looked like a cop standing there with my name out
I ain't trying to get arrested yet
I just got here
I sprang with the quickness like lightning, disappeared
I whistled for a cab and when it came near
The license plate said fresh and it had dice in the mirror
If anything I could say that this cab was rare
But I thought 'Nah, forget it' - 'Yo, homes to Bel Air'
I pulled up to the house about seven or eigth
And I yelled to the cabbie 'Yo homes smell ya later'
I looked at my kingdom
I was finally there
To sit on my throne as the Prince of Bel Air)100" };
	ULONG sizeDataUncompressed = compressThis.size() * sizeof(char);
	ULONG sizeDataCompressed = sizeDataUncompressed * 1.1 + 12;
	BYTE* dataCompressed = new BYTE[sizeDataCompressed];
	int z_result = compress(
		dataCompressed,
		&sizeDataCompressed,
		(BYTE*)compressThis.c_str(),
		compressThis.size()
	);
	if (z_result != Z_OK)
		printf("FAILURE!!!!!!!!!!!!!!!!!!!!!\n");
	printf("Uncompressed size: %d\n", sizeDataUncompressed);
	printf("Compressed size: %d\n", sizeDataCompressed);
	printf("%s", compressThis.c_str());
	for (int i = 0; i < sizeDataCompressed; i++)
		putchar(dataCompressed[i]);
	printf("\n--\n\n");

	printf("Uncompressing data...\n");
	BYTE* dataUncompressed = new BYTE[sizeDataUncompressed];
	z_result = uncompress(
		dataUncompressed,
		&sizeDataUncompressed,
		dataCompressed,
		sizeDataCompressed
	);
	if (z_result != Z_OK)
		printf("FAILURE!!!!!!!!!!!!!!!!!!!!!\n");
	printf("Uncompressed, size = %d\n", sizeDataUncompressed);
	for (int i = 0; i < sizeDataUncompressed; i++)
		putchar(dataUncompressed[i]);

	assert(strcmp((char*)dataUncompressed, compressThis.c_str()) == 0);
	delete[] dataCompressed;
	delete[] dataUncompressed;
}

int main()
{
	ENetAddress address;
	ENetHost* server;
	ENetEvent event;
	int eventStatus;

	// a. Initialize enet
	if (enet_initialize() != 0)
	{
		printf("An error occured while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	
	atexit(enet_deinitialize);

	// b. Create a host using enet_host_create
	address.host = ENET_HOST_ANY;
	address.port = 1234;

	server = enet_host_create(&address, 32, 2, 0, 0);

	if (server == NULL)
	{
		printf("An error occured while trying to create an ENet server host\n");
		exit(EXIT_FAILURE);
	}

	// c. Connect and user service
	eventStatus = 1;

	printf("Server started successfully!");
	while (1)
	{
		eventStatus = enet_host_service(server, &event, 1000);

		// If we had some event that interested us
		if (eventStatus > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				printf("(Server) We got a new connection from %x\n",
					event.peer->address.host);
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				//printf("(Server) Message from client : %s\n", event.packet->data);
				//// Lets broadcast this message to all
				//enet_host_broadcast(server, 0, event.packet);

				// we are guaranteed to receive at least the type identifier
				assert(event.packet->dataLength >= sizeof(int));

				Net::Packet packet;

				// assume first 4 bytes are size
				std::memcpy(&packet.type, event.packet->data, sizeof(int));
				
				// 'data' points to region directly after type identifier
				packet.data = event.packet->data + sizeof(int);
				Net::ProcessClientEvent(packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("%s disconnected.\n", event.peer->data);

				// Reset client's information
				event.peer->data = NULL;
				break;
			}
		}
	}
}