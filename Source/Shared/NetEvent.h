#pragma once
#include <varargs.h>
#include <queue>
#include <shared_mutex>

#define REGISTER_EVENT(x) x* x ## _data
//#define PROCESS_EVENT(x) case Packet::e ## x: data.x ## _data->Process(); break
#define UNDEFINED_EVENT(x) ;

namespace Net
{
	struct Packet
	{
		enum ClientEvent
		{
			/* Client Join Event
				Sent to the server upon initial connection.
				The server will return a result stating whether the
				connection was accepted or declined. This is different
				from simply connecting to the server's socket, as 
				this event will give the server discretionary control 
				over what clients may or may not join.
			*/
			eClientJoinEvent = 1,

			/* Client Leave Event
				Sent to server before client disconnects. No acknowledgement
				required. This is purely for the server's convenience.
				The server will disconnect the client forcibly if their timeout
				limit is reached.
			*/
			eClientLeaveEvent,

			/* Client Print vec3 Event
				Test event. When sent, the server will print the value
				of the vec3 contained within.
			*/
			eClientPrintVec3Event,

			/* Client Input
				Sent after polling inputs each client tick.
				Aggregate of all input actions a client can take in a frame.
				Booleans packed into bytes indicating whether an action is taken.
			*/
			eClientInput,
		};

		enum ServerEvent
		{
			/* Server Join Result Event
				Sent to a particular client after they sent a ClientJoinEvent.
				The server will inform the client if their connection request
				was successful and, if it was, what their assigned client ID is.
			*/
			eServerJoinResultEvent = 1,

			/* Server Game State
				Broadcast to all clients each server tick.
				Contains dynamical visible information, such as player and mob
				positions, or player-held items.
			*/
			eServerGameState,
		};

		// A client OR server event type
		int type = 0;
		void* data = nullptr;
	};

	struct ClientJoinEvent
	{
	};

	struct ClientPrintVec3Event
	{
		int clientID;
		glm::vec3 v;
	};


	struct ClientInput
	{
		// TODO: pack the inputs
		int clientID;
		bool moveFoward;
		bool moveBack;
		bool moveLeft;
		bool moveRight;
		bool jump;
	};


	// concurrent structure for processing outgoing network events
	struct EventController
	{
	public:
		void PushEvent(Packet&& p)
		{
			mtx.lock();
			backBuffer.push(std::move(p));
			mtx.unlock();
		}

		Packet PopEvent()
		{
			mtx.lock();
			Packet&& t = std::move(packetBuf.front());
			packetBuf.pop();
			mtx.unlock();
			return t;
		}

		void SwapBuffers()
		{
			mtx.lock();
			packetBuf.swap(backBuffer);
			mtx.unlock();
		}

	private:
		std::queue<Packet> packetBuf;
		std::queue<Packet> backBuffer;
		std::shared_mutex mtx;
	};
}