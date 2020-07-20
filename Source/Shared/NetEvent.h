#pragma once
#include <queue>
#include <shared_mutex>
#include <enet/enet.h>

#define REGISTER_EVENT(x) x* x ## _data
//#define PROCESS_EVENT(x) case Packet::e ## x: data.x ## _data->Process(); break
#define UNDEFINED_EVENT(x) ;

namespace Net
{
	// a packet that is ready to be sent and interpreted over a network
	struct Packet
	{
		// constructing a packet
		Packet(int type, void* data, size_t data_size)
		{
			buf = new std::byte[sizeof(int) + data_size];
			reinterpret_cast<int*>(buf)[0] = type;
			std::memcpy(buf + sizeof(int), data, data_size);
		}

		// interpreting an enet packet
		// TODO: make this constructor it's own class that doesn't require copying data
		Packet(const ENetPacket* ev)
		{
			buf = new std::byte[ev->dataLength];
			std::memcpy(buf, ev->data, ev->dataLength);
		}

		~Packet()
		{
			delete[] buf;
		}

		int GetType() { return reinterpret_cast<int*>(buf)[0]; }
		std::byte* GetData() { return buf + sizeof(int); }
		std::byte* GetBuffer() { return buf; }

	private:
		// A client OR server event type
		//int type = -1;
		//void* data = nullptr;
		std::byte* buf;
	public:
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
			eClientJoinEvent,

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
			eServerJoinResultEvent,

			/* Server List Players
				Broadcast to every client when a player joins or disconnects from
				the server.
			*/
			eServerListPlayersEvent,

			/* Server Game State
				Broadcast to all clients each server tick.
				Contains dynamical visible information, such as player and mob
				positions, or player-held items.
			*/
			eServerGameState,
		};
	};

	//struct ClientJoinEvent
	//{
	//};

	struct ClientPrintVec3Event
	{
		glm::vec3 v;
	};

	struct ServerJoinResultEvent
	{
		bool success;
		int id; // uninitialized if success is false
	};

	struct ServerListPlayersEvent
	{
		int connected;
		int* IDs;
	};

	struct ClientInput
	{
		// TODO: pack the inputs, perhaps into a bitfield?
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





#if 0
// below is some weird experimental garbage that I made for fun :)
// DON'T ACTUALLY USE THIS CLASS!
// TODO: add bits to this
template<class ... Names>
struct BoolField
{
	template<unsigned I>
	struct Proxy
	{
		static_assert(I < 8, "BoolField too large!");

		Proxy(uint8_t& data) : data_(data) {}
		Proxy(const Proxy& other) : data_(other.data_) {}
		
		operator bool() { return data_ & (1 << I); };
		Proxy<I>& operator=(bool rhs) { data_ |= rhs << I; }

	private:
		uint8_t& data_;
	};

	template<class Name>
	auto Get() // TODO: remove
	{
		return Proxy<getIndex(Name)>(data_);
	}

private:
	consteval int getIndex(auto name)
	{
		int i = 0;
		for (const auto N : { Names... })
		{
			if constexpr (name == N)
				return i;
			++i;
		}

		static_assert(0, "Failed to find member name in template args!");
	}

	uint8_t data_;
};
#endif