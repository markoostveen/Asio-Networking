#pragma once

#include "config.h"

#include "PeerConnection.h"

#include "asio.hpp"

#include <vector>
#include <string>
#include <iostream>

namespace Networking {
	class Server {
	public:
		Server(const Server&) = delete;
		Server() = delete;

		void WaitForIncomingConnection();
		bool Connect(const std::string& host, const int port);

		uint32_t ConnectedPeerCount() {
			return _peers.size();
		}

		const PeerConnection* GetPeer(uint32_t index) {
			return _peers[index].get();
		}

		short Port() {
			return _acceptor.local_endpoint().port();
		}

		void Disconnect(const PeerConnection* peer) {
			for (int i = 0; i < _peers.size(); i++)
			{
				if (_peers[i].get() == peer) {
					_peers.erase(_peers.begin() + i);
					break;
				}
			}
		}

	protected:
		Server(asio::io_context& io_context, short port)
			: _io_context(io_context), _acceptor(_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port), port)
		{
			std::cout << "Server running on " << _acceptor.local_endpoint().address() << ":" << port << std::endl;
		}

		virtual bool OnPeerConnected(PeerConnection* newPeer) = 0;

	private:

		PeerConnection* AddPeer(asio::ip::tcp::socket socket);
		asio::io_context& _io_context;
		asio::ip::tcp::acceptor _acceptor;
		std::vector<std::unique_ptr<PeerConnection>> _peers;
	};
}