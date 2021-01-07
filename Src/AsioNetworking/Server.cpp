#include "Server.h"
#include "Message.h"

#include "ServerCategoryHandler.h"

#include <iostream>

namespace Networking {

	void Server::RemoveCategoryHandler(uint8_t categoryId)
	{
		_categoryHandlers.erase(categoryId);
	}

	CategorizedConnectionHandlerBase* Server::GetCategoryHandler(uint8_t categoryId)
	{
		auto it = _categoryHandlers.find(categoryId);
		if (it != _categoryHandlers.end()) {
			return it->second.get(); // found the correct handler
		}

		return nullptr;
	}

	void Server::WaitForIncomingConnection()
	{
		_acceptor.async_accept([this](std::error_code errorCode, asio::ip::tcp::socket socket) {
			if (!errorCode)
			{
				std::string address = socket.remote_endpoint().address().to_string();

				// Create a new connection to handle this client
				PeerConnection* peer = AddPeer(std::move(socket));

				if (OnPeerConnected(peer)) {
					std::cout << "Connection approved: " << address << std::endl;

					ServerCategoryHandler* serverHandler = GetCategoryHandler<ServerCategoryHandler>();
					serverHandler->SendWelcomeMessage(peer); // give client opertunity to send important message
					serverHandler->SendRequestServerPort(peer); // ask for the identifier of their server
					serverHandler->SendRequestPeerList(peer); // ask for their peer list so we can also make connections
				}
				else {
					Disconnect(peer);
				}
			}
			else
			{
				std::cout << "Connection attempt: " << errorCode.message() << "\n";
			}

			WaitForIncomingConnection();
			});
	}

	bool Server::Connect(const std::string& host, const int port)
	{
		try
		{
			std::cout << "Connecting to peer at " << host << ":" << port << std::endl;
			asio::ip::tcp::resolver resolver(_io_context);
			asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

			auto socket = std::make_shared< asio::ip::tcp::socket>(_io_context);

			asio::async_connect(*socket, endpoints,
				[this, port, socket](std::error_code errorCode, asio::ip::tcp::endpoint endpoint)
				{
					if (!errorCode)
					{
						PeerConnection* peer = AddPeer(std::move(*socket));
						peer->SetOriginalPort(port);
						if (OnPeerConnected(peer)) {
							std::cout << "Connected to " << endpoint << std::endl;
						}
						else {
							Disconnect(peer);
						}
					}
				});
		}
		catch (std::exception& exception)
		{
			std::cerr << "Client Exception: " << exception.what() << std::endl;
			return false;
		}
		return true;
	}

	uint32_t Server::ConnectedPeerCount()
	{
		return _peers.size();
	}

	PeerConnection* Server::GetPeer(uint32_t id)
	{
		for (size_t i = 0; i < _peers.size(); i++)
		{
			if (_peers[i]->Id() == id)
				return _peers[i].get();
		}

		return nullptr;
	}

	uint32_t Server::GetPeerId(uint32_t index)
	{
		return _peers[index]->Id();
	}

	short Server::Port()
	{
		return _acceptor.local_endpoint().port();
	}

	void Server::RefreshConnectedPeers()
	{
		std::vector<PeerConnection*> disconnectedPeers;
		for (int i = 0; i < _peers.size(); i++)
		{
			PeerConnection* peer = _peers[i].get();
			if (!peer->IsConnected())
				disconnectedPeers.push_back(peer);
		}

		for (int i = 0; i < disconnectedPeers.size(); i++)
		{
			Disconnect(disconnectedPeers[i]);
		}
	}

	void Server::Disconnect(const PeerConnection* peer)
	{
		for (int i = 0; i < _peers.size(); i++)
		{
			if (_peers[i].get() == peer) {
				std::cout << "Disconnected from peer because of a closed socket" << std::endl;
				_peers.erase(_peers.begin() + i);
				break;
			}
		}
	}

	Server::Server(asio::io_context& io_context, short port)
		: _io_context(io_context), _acceptor(_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port), port)
	{
		std::shared_ptr<ServerCategoryHandler> serverHandler = std::make_shared<ServerCategoryHandler>(this);
		AddCategoryHandler(serverHandler);

		std::cout << "Server running on " << _acceptor.local_endpoint().address() << ":" << port << std::endl;
	}

	PeerConnection* Server::AddPeer(asio::ip::tcp::socket socket)
	{
		int index = _peers.size();
		_peers.push_back(std::make_unique<PeerConnection>(this, _io_context, std::move(socket)));
		return _peers[index].get();
	}
}