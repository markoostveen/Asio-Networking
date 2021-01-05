#include "CategorizedConnectionHandler.h";
#include "Server.h"

void Networking::CategorizedConnectionHandler::SendMessageToPeer(PeerConnection* peer, Message& message) {
	if (peer->IsConnected()) {
		message.Header.SetCategory(CategoryId);
		peer->SendMessageToPeer(message);
	}
	else {
		server->Disconnect(peer); // there is no more connection with the client
	}
}