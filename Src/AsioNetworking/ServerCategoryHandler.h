#pragma once

#include "CategorizedConnectionHandler.h"
#include "PeerConnection.h"

namespace Networking {
	class ServerCategoryHandler : public CategorizedConnectionHandler<0> {
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
			: CategorizedConnectionHandler(parentServer) {}

		void ProcessMessage(PeerConnection* peer, Message& message) final;

		void ReceiveWelcomeMessage(PeerConnection* peer, Message& message);

		void SendWelcomeMessage(PeerConnection* peer);

		void ReceiveServerPort(PeerConnection* peer, Message& message);
		void SendRequestServerPort(PeerConnection* peer);

		void SendServerPort(PeerConnection* peer);

		void SendRequestPeerList(PeerConnection* peer);

		void SendPeerList(PeerConnection* peer);

		void ReceivePeerList(Message& message);
	};
}