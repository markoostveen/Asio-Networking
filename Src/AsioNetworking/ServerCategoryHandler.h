#pragma once

#include "CategorizedConnectionHandler.h"
#include "PeerConnection.h"

namespace Networking {
	constexpr uint8_t ServerCategory = 0;

	class ServerCategoryHandler : public CategorizedConnectionHandler {
	private:
		enum Messages {
			WelcomeMessage = 0,
			RequestPeerList,
			PeerList
		};

	public:
		ServerCategoryHandler(Server* parentServer)
			: CategorizedConnectionHandler(ServerCategory, parentServer) {}

		void ProcessMessage(PeerConnection& peer, Message& message) final;

		void ReceiveWelcomeMessage(PeerConnection& peer, Message& message);

		void SendWelcomeMessage(PeerConnection& peer);

		void SendRequestPeerList(PeerConnection& peer);

		void ReceiveRequestPeerList(PeerConnection& peer, Message& message);

		void ReceivePeerList(Message& message);
	};
}