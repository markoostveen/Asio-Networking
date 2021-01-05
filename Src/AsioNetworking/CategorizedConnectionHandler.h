#pragma once

#include "Message.h"
#include "PeerConnection.h"

namespace Networking {
	class Server;

	/// <summary>
	/// This class is intended to be used by users to send messages to clients and process the returned messages from clients
	/// These CategorizedConnection objects are automatically setting the category id when sending a message, and they get messages with their category back from the PeerConnection
	/// </summary>
	class CategorizedConnectionHandler {
	public:

		virtual void ProcessMessage(PeerConnection& peer, Message& message) = 0;

		uint8_t CategoryId;

	protected:
		CategorizedConnectionHandler(uint8_t categoryId, Server* parentServer)
			: CategoryId(categoryId), server(parentServer) {}

		void SendMessageToPeer(PeerConnection& peer, Message& message) {
			message.Header.SetCategory(CategoryId);
			peer.SendMessageToPeer(message);
		}

		Server* server;
	};
}
