#include "ServerCategoryHandler.h"
#include "Server.h"

#include <string>
#include <iostream>

struct WelcomeMessageData {
	uint8_t WelcomeStringSize;
};

struct PeerListData {
	uint8_t PeerCount;
};

void Networking::ServerCategoryHandler::ProcessMessage(PeerConnection& peer, Message& message)
{
	switch (message.Header.Type) {
	case Messages::WelcomeMessage:
		ReceiveWelcomeMessage(peer, message);
		break;
	case Messages::RequestPeerList:
		ReceiveRequestPeerList(peer, message);
		break;
	case Messages::PeerList:
		ReceivePeerList(message);
		break;
	default:
		std::cout << "No implementation exists for this message." << std::endl;
		break;
	}
}

void Networking::ServerCategoryHandler::ReceiveWelcomeMessage(PeerConnection& peer, Message& message)
{
	WelcomeMessageData& messageData = message.GetData<WelcomeMessageData>();
	std::string messageString;
	messageString.resize(messageData.WelcomeStringSize);
	message.Pull(messageString.data(), messageData.WelcomeStringSize);

	std::cout << messageString << std::endl;
	SendRequestPeerList(peer);
}

void Networking::ServerCategoryHandler::SendWelcomeMessage(PeerConnection& peer)
{
	std::stringstream welcomeMessageStream;
	welcomeMessageStream << "Welcome, you are now connected to " << server->LocalAddress();
	std::string welcomeMessageString = welcomeMessageStream.str();
	WelcomeMessageData messageData;
	messageData.WelcomeStringSize = welcomeMessageString.size();

	Message message = Message::CreateMessage(Messages::WelcomeMessage);
	message.Push(messageData);
	message.Push(welcomeMessageString.data(), messageData.WelcomeStringSize);

	SendMessageToPeer(peer, message);
}

void Networking::ServerCategoryHandler::SendRequestPeerList(PeerConnection& peer)
{
	Message message = Message::CreateMessage(Messages::RequestPeerList);

	SendMessageToPeer(peer, message);
}

void Networking::ServerCategoryHandler::ReceiveRequestPeerList(PeerConnection& peer, Message& message)
{
	Message returnMessage = Message::CreateMessage(Messages::PeerList);

	PeerListData peerListData;
	peerListData.PeerCount = server->ConnectedPeerCount() - 1; // minus to exclude calling peer

	returnMessage.Push(peerListData);

	// determin length of address strings, and store in array
	uint16_t peerAddressCharCount[peerListData.PeerCount];
	for (uint16_t i = 0; i < peerListData.PeerCount; i++)
	{
		const PeerConnection& serverPeer = server->GetPeer(i);
		if (&peer == &serverPeer)
			continue; // excluding calling peer

		std::string address = serverPeer.Address();
		peerAddressCharCount[i] = address.size();
		// push actual address
		returnMessage.Push(address.data(), peerAddressCharCount[i]);
	}

	// push information
	returnMessage.Push(peerAddressCharCount, sizeof(uint16_t[peerListData.PeerCount]));

	SendMessageToPeer(peer, returnMessage);
}

void Networking::ServerCategoryHandler::ReceivePeerList(Message& message)
{
	PeerListData messageData = message.GetData<PeerListData>();

	uint16_t peerAddressLengths[messageData.PeerCount];
	message.Pull(peerAddressLengths, sizeof(uint16_t[messageData.PeerCount]));

	for (int i = messageData.PeerCount - 1; i >= 0; i--)
	{
		std::string address;
		address.resize(peerAddressLengths[i]);
		message.Pull(address.data(), peerAddressLengths[i]);

		std::cout << "Received peer address: " << address << std::endl;
	}

	std::cout << "Received peer list with " << std::to_string(messageData.PeerCount) << " peers" << std::endl;
}