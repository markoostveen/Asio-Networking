#pragma once

#include "config.h"

#include "PeerConnection.h"

#include "asio.hpp"

#include <vector>
#include <string>

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

		const PeerConnection& GetPeer(uint32_t index) {
			return _peers[index];
		}

		std::string LocalAddress() {
			std::stringstream ss;
			ss << _acceptor.local_endpoint().address().to_string();
			ss << ":";
			ss << _acceptor.local_endpoint().port();
			return ss.str();
		}

	protected:
		Server(asio::io_context& io_context, short port) : _io_context(io_context), _acceptor(_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port), port) {}

		virtual bool OnPeerConnected(PeerConnection& newPeer) = 0;

	private:

		PeerConnection& AddPeer(asio::ip::tcp::socket socket);
		asio::io_context& _io_context;
		asio::ip::tcp::acceptor _acceptor;
		std::vector<PeerConnection> _peers;
	};
}