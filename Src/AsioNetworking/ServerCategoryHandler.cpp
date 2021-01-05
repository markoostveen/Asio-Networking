#include "ServerCategoryHandler.h"
#include "Server.h"

#include <string>
#include <iostream>
#include <vector>
#include <unordered_set>

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
	case Messages::RequestServerPort:
		SendServerPort(peer);
		break;
	case Messages::ServerPort:
		ReceiveServerPort(peer, message);
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

	std::cout << messageString << "[" << peer.Address() << "]" << std::endl;
	SendRequestPeerList(peer);
}

void Networking::ServerCategoryHandler::SendWelcomeMessage(PeerConnection& peer)
{
	std::string welcomeMessageString = "Welcome hello from your new buddy";
	WelcomeMessageData messageData;
	messageData.WelcomeStringSize = welcomeMessageString.size();

	Message message = Message::CreateMessage(Messages::WelcomeMessage);
	message.Push(messageData);
	message.Push(welcomeMessageString.data(), messageData.WelcomeStringSize);

	SendMessageToPeer(peer, message);
}

void Networking::ServerCategoryHandler::SendRequestServerPort(PeerConnection& peer)
{
	Message message = Message::CreateMessage(Messages::RequestServerPort);
	SendMessageToPeer(peer, message);
}

void Networking::ServerCategoryHandler::SendServerPort(PeerConnection& peer)
{
	Message message = Message::CreateMessage(Messages::ServerPort);
	short port = server->Port();
	message.Push(port);

	SendMessageToPeer(peer, message);
}

void Networking::ServerCategoryHandler::ReceiveServerPort(PeerConnection& peer, Message& message)
{
	short port;
	message.Pull(port);
	peer.SetOriginalPort(port);
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

	std::vector<std::string> peerAddresses;
	std::unordered_set<std::string> peerAddressesSet;

	for (int i = messageData.PeerCount - 1; i >= 0; i--)
	{
		std::string address;
		address.resize(peerAddressLengths[i]);
		message.Pull(address.data(), peerAddressLengths[i]);
		peerAddresses.emplace_back(address);
		peerAddressesSet.emplace(address);

		std::cout << "Received peer address: " << address << std::endl;
	}

	std::cout << "Received peer list with " << std::to_string(messageData.PeerCount) << " peers" << std::endl;

	for (int i = 0; i < peerAddresses.size(); i++)
	{
		const PeerConnection& peer = server->GetPeer(i);
		std::string address = peer.Address();
		auto it = peerAddressesSet.find(address);
		if (it == peerAddressesSet.end()) {
			// Found a new peer that has not been identified before
			// use address and our own configured port to try and connect
			std::cout << "Connecting to peer at " << address << ":" << server->Port() << std::endl;

			address = peerAddresses[i];
			std::string ipAddress = address.substr(0, address.find_first_of(':'));
			std::string port = address.substr(address.find_first_of(':') + 1, address.size());
			server->Connect(address, std::stoi(port));
		}
	}
}