#include "stdafx.h"

#include "Server.h"
#include <iostream>

int main()
{
	// start the network thread
	Net::Server server;
	bool result = server.Init();
	if (result == false)
		return EXIT_FAILURE;

	// begin an interactive session
	printf("Welcome to the interactive server console!\n");
	printf("Input a command.\n");
	while (1)
	{
		std::cout << ": ";
		std::string in;
		std::cin >> in;
		std::cout << "You wrote: " << in;
		std::cout << std::endl;

		if (in == "q" || in == "quit" || in == "exit")
			break;
	}

	server.Shutdown();
}