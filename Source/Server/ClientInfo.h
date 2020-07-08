#pragma once

struct ClientInfo
{
	int clientID = -1;
	double time_since_heard = 0;

	struct
	{
		glm::vec3 pos;
		glm::vec3 front;
	}playerData;
};