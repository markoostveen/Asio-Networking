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

void Networking::ServerCategoryHandler::ProcessMessage(PeerConnection* peer, Message& message)
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
		SendPeerList(peer);
		break;
	case Messages::PeerList:
		ReceivePeerList(message);
		break;
	default:
		std::cout << "No implementation exists for this message." << std::endl;
		break;
	}
}

void Networking::ServerCategoryHandler::ReceiveWelcomeMessage(PeerConnection* peer, Message& message)
{
	WelcomeMessageData& messageData = message.GetData<WelcomeMessageData>();
	std::string messageString;
	messageString.resize(messageData.WelcomeStringSize);
	message.Pull(messageString.data(), messageData.WelcomeStringSize);

	std::cout << messageString << "[" << peer->Address() << "]" << std::endl;
}

void Networking::ServerCategoryHandler::SendWelcomeMessage(PeerConnection* peer)
{
	std::string welcomeMessageString = "Welcome hello from your new buddy";
	WelcomeMessageData messageData;
	messageData.WelcomeStringSize = welcomeMessageString.size();

	Message message = Message::CreateMessage(Messages::WelcomeMessage);
	message.Push(messageData);
	message.Push(welcomeMessageString.data(), messageData.WelcomeStringSize);

	SendMessageToPeer(peer, message);
}

void Networking::ServerCategoryHandler::SendRequestServerPort(PeerConnection* peer)
{
	Message message = Message::CreateMessage(Messages::RequestServerPort);
	SendMessageToPeer(peer, message);
}

void Networking::ServerCategoryHandler::SendServerPort(PeerConnection* peer)
{
	Message message = Message::CreateMessage(Messages::ServerPort);
	short port = server->Port();
	message.Push(port);

	SendMessageToPeer(peer, message);
}

void Networking::ServerCategoryHandler::ReceiveServerPort(PeerConnection* peer, Message& message)
{
	short port;
	message.Pull(port);
	peer->SetOriginalPort(port);
}

void Networking::ServerCategoryHandler::SendRequestPeerList(PeerConnection* peer)
{
	Message message = Message::CreateMessage(Messages::RequestPeerList);

	SendMessageToPeer(peer, message);
}

void Networking::ServerCategoryHandler::SendPeerList(PeerConnection* peer)
{
#ifdef NetworkingDebug
	std::cout << "Sending peer list to " << peer->Address() << ":" << std::endl;
#endif

	Message returnMessage = Message::CreateMessage(Messages::PeerList);

	if (!peer->IsConnected())
		return;

	server->RefreshConnectedPeers(); // make sure peer list is accurate

	PeerListData peerListData;
	peerListData.PeerCount = server->ConnectedPeerCount() - 1; // minus to exclude calling peer

	if (peerListData.PeerCount < 0)
		peerListData.PeerCount = 0;

	returnMessage.Push(peerListData);

	// determin length of address strings, and store in array
	std::unique_ptr<uint16_t[]> peerAddressCharCount = std::make_unique<uint16_t[]>(peerListData.PeerCount);
	int bufferIndex = 0;
	for (uint16_t i = 0; i < server->ConnectedPeerCount(); i++)
	{
		uint32_t serverPeerID = server->GetPeerId(i);
		PeerConnection* serverPeer = server->GetPeer(serverPeerID);
		if (peer == serverPeer)
			continue; // excluding calling peer

		std::string address = server->GetPeer(serverPeerID)->Address();
		peerAddressCharCount[bufferIndex] = address.size();
		// push actual address
		returnMessage.Push(address.data(), peerAddressCharCount.get()[bufferIndex]);
		bufferIndex++;
	}

	// push information
	returnMessage.Push(peerAddressCharCount.get(), sizeof(uint16_t) * peerListData.PeerCount);

	SendMessageToPeer(peer, returnMessage);
}

void Networking::ServerCategoryHandler::ReceivePeerList(Message& message)
{
	PeerListData messageData = message.GetData<PeerListData>();

	std::unique_ptr<uint16_t[]> peerAddressLengths = std::make_unique<uint16_t[]>(messageData.PeerCount);
	message.Pull(peerAddressLengths.get(), sizeof(uint16_t) * messageData.PeerCount);

	std::unordered_set<std::string> peerAddressesSet;

	for (int i = messageData.PeerCount - 1; i >= 0; i--)
	{
		uint16_t stringSize = peerAddressLengths.get()[i];
		std::string address;
		address.resize(stringSize);
		message.Pull(address.data(), stringSize);
		peerAddressesSet.emplace(address);

		//std::cout << "Received peer address(" << stringSize << "): " << address << std::endl;
	}

	server->RefreshConnectedPeers(); // make sure connected peer list is accurate

	// remove all addresses we can't connect to because of duplicate
	for (int i = 0; i < server->ConnectedPeerCount(); i++)
	{
		PeerConnection* peer = server->GetPeer(server->GetPeerId(i));
		std::string address = peer->Address();

		auto it = peerAddressesSet.find(peer->Address());
		if (it != peerAddressesSet.end()) {
			peerAddressesSet.erase(*it);
		}
	}

#ifdef NetworkingDebug
	if (peerAddressesSet.size() > 0)
		std::cout << "Received peer list with " << std::to_string(messageData.PeerCount) << " peers" << std::endl;
#endif

	for (auto& address : peerAddressesSet)
	{
		// Found a new peer that has not been identified before or has disconnected and we try to reconnect
		// use address and our own configured port to try and connect

		std::string ipAddress = address.substr(0, address.find_first_of(':'));
		std::string port = address.substr(address.find_first_of(':') + 1, address.size());
		server->Connect(ipAddress, std::stoi(port));
	}
}