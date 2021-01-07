#pragma once

#include "Message.h"
#include "PeerConnection.h"

namespace Networking {
	class Server;

	class CategorizedConnectionHandlerBase {
	public:
		virtual void ProcessMessage(PeerConnection* peer, Message& message) = 0;
	};


	/// <summary>
	/// This class is intended to be used by users to send messages to clients and process the returned messages from clients
	/// These CategorizedConnection objects are automatically setting the category id when sending a message, and they get messages with their category back from the PeerConnection
	/// </summary>
	template<uint8_t CategoryId>
	class CategorizedConnectionHandler : public CategorizedConnectionHandlerBase {
	public:
		constexpr static uint8_t ID() {
			return CategoryId;
		}


	protected:
		CategorizedConnectionHandler(Server* parentServer)
			: server(parentServer) {
			UsageCount++;
			assert(UsageCount == 1, "ID has been used multiple times, making it invalid");
		}

		virtual ~CategorizedConnectionHandler() {
			UsageCount--;
		}


		void SendMessageToPeer(PeerConnection* peer, Message& message) {
			if (peer->IsConnected()) {
				message.Header.SetCategory(CategoryId);
				peer->SendMessageToPeer(message);
			}
			else {
				server->Disconnect(peer); // there is no more connection with the client
			}
		}

		Server* server;
		inline static uint8_t UsageCount;
	};
}
