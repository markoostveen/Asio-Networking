#pragma once

#include "config.h"
#include "Message.h"

#include "asio.hpp"

#include <unordered_map>
#include <queue>

namespace Networking {
	class Server;

	class PeerConnection {
	public:
		PeerConnection(Server* server, asio::io_context& context, asio::ip::tcp::socket socket);

		void SendMessageToPeer(Message message);

		std::string Address() const {
			auto endpoint = _socket.remote_endpoint();
			std::stringstream ss;
			ss << endpoint.address().to_string();
			ss << ":";
			ss << _originalPort;
			return ss.str();
		}

		void SetOriginalPort(short port) {
			_originalPort = port;
		}

		bool IsConnected() const {
			return _socket.is_open();
		}

		uint32_t Id() {
			return _id;
		}

	private:
		void ReadHeader();
		void ReadBody();
		void WriteHeader();
		void WriteBody();

		void ProcessReceivedMessage(Message& message);

		asio::io_context& _io_context;
		asio::ip::tcp::socket _socket;
		short _originalPort; // remote server port of peer
		const uint32_t _id; // local identifier
		
		Server* _server;

		// messages
		std::queue<Message> _outgoingMessages{};

		//buffers
		Message _readBuffer{};
	};
}
