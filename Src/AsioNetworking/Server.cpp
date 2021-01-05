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
				std::cout << "New Connection: " << socket.remote_endpoint() << "\n";

				// Create a new connection to handle this client
				PeerConnection& peer = AddPeer(std::move(socket));

				if (OnPeerConnected(peer)) {
					std::shared_ptr<ServerCategoryHandler> serverHandler = std::make_shared<ServerCategoryHandler>(this);
					peer.AddCategoryCallback(ServerCategory, serverHandler);
					serverHandler->SendWelcomeMessage(peer);
					serverHandler->SendRequestServerPort(peer);
				}
			}
			else
			{
				std::cout << "New connection error: " << errorCode.message() << "\n";
			}

			WaitForIncomingConnection();
			});
	}

	bool Server::Connect(const std::string& host, const int port)
	{
		try
		{
			asio::ip::tcp::resolver resolver(_io_context);
			asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

			auto socket = std::make_shared< asio::ip::tcp::socket>(_io_context);

			asio::async_connect(*socket, endpoints,
				[this, port, socket](std::error_code ec, asio::ip::tcp::endpoint endpoint)
				{
					if (!ec)
					{
						PeerConnection& peer = AddPeer(std::move(*socket));
						peer.SetOriginalPort(port);
						if (OnPeerConnected(peer)) {
							std::shared_ptr<ServerCategoryHandler> serverHandler = std::make_shared<ServerCategoryHandler>(this);
							peer.AddCategoryCallback(ServerCategory, serverHandler);
						}
					}
				});
		}
		catch (std::exception& e)
		{
			std::cerr << "Client Exception: " << e.what() << "\n";
			return false;
		}
		return true;
	}

	PeerConnection& Server::AddPeer(asio::ip::tcp::socket socket)
	{
		return _peers.emplace_back(_io_context, std::move(socket));
	}
}