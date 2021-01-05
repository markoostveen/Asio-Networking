#include "Server.h"
#include "Message.h"

#include "ServerCategoryHandler.h"

#include <iostream>

namespace Networking {
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

					std::shared_ptr<ServerCategoryHandler> serverHandler = std::make_shared<ServerCategoryHandler>(this);
					peer->AddCategoryCallback(ServerCategory, serverHandler);
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

							std::shared_ptr<ServerCategoryHandler> serverHandler = std::make_shared<ServerCategoryHandler>(this);
							peer->AddCategoryCallback(ServerCategory, serverHandler);
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

	PeerConnection* Server::AddPeer(asio::ip::tcp::socket socket)
	{
		int index = _peers.size();
		_peers.push_back(std::make_unique<PeerConnection>(_io_context, std::move(socket)));
		return _peers[index].get();
	}
}