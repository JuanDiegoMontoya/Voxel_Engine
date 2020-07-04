#pragma once

struct Packet;

namespace Net
{
	void ProcessClientEvent(Packet& packet);
}