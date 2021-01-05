#pragma once

#include "CategorizedConnectionHandler.h"
#include "PeerConnection.h"

namespace Networking {
	constexpr uint8_t ServerCategory = 0;

	class ServerCategoryHandler : public CategorizedConnectionHandler {
	private:
		enum Messages {
			WelcomeMessage = 0,
			RequestServerPort,
			ServerPort,
			RequestPeerList,
			PeerList
		};

	public:
		ServerCategoryHandler(Server* parentServer)
			: CategorizedConnectionHandler(ServerCategory, parentServer) {}

		void ProcessMessage(PeerConnection& peer, Message& message) final;

		void ReceiveWelcomeMessage(PeerConnection& peer, Message& message);

		void SendWelcomeMessage(PeerConnection& peer);

		void ReceiveServerPort(PeerConnection& peer, Message& message);
		void SendRequestServerPort(PeerConnection& peer);

		void SendServerPort(PeerConnection& peer);

		void SendRequestPeerList(PeerConnection& peer);

		void ReceiveRequestPeerList(PeerConnection& peer, Message& message);

		void ReceivePeerList(Message& message);
	};
}