// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <enet/enet.h>

int main()
{
	if (enet_initialize() != 0)
	{
		std::cout << "An error occured while initializing ENet.\n";
		return EXIT_FAILURE;
	}
	atexit(enet_deinitialize);


	ENetAddress address;
	ENetHost* server;
	/* Bind the server to the default localhost.     */
	/* A specific host address can be specified by   */
	/* enet_address_set_host (& address, "x.x.x.x"); */
	address.host = ENET_HOST_ANY;
	/* Bind the server to port 1234. */
	address.port = 1234;
	server = enet_host_create(&address /* the address to bind the server host to */,
		32      /* allow up to 32 clients and/or outgoing connections */,
		2       /* allow up to 2 channels to be used, 0 and 1 */,
		0       /* assume any amount of incoming bandwidth */,
		0       /* assume any amount of outgoing bandwidth */);
	if (server == NULL)
	{
		std::cout << "An error occurred while trying to create an ENet server host.\n";
		exit(EXIT_FAILURE);
	}
	
	enet_host_destroy(server);
	std::cout << "End of program. Success?\n";
}