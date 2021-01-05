#pragma once

#include "config.h"
#include "Message.h"

#include "asio.hpp"

#include <unordered_map>
#include <queue>

namespace Networking {
	class CategorizedConnectionHandler;

	class PeerConnection {
	public:
		typedef void (*CategoryCallback)(Message& message);

		PeerConnection(asio::io_context& context, asio::ip::tcp::socket socket) : _io_context(context), _socket(std::move(socket)) {
			ReadHeader(); // start polling for new messages
		}

		void SendMessageToPeer(Message message);

		void AddCategoryCallback(uint8_t categoryId, std::shared_ptr<CategorizedConnectionHandler> categoryHandler);
		void RemoveCategoryCallback(uint8_t categoryId);

		std::string Address() const {
			auto endpoint = _socket.remote_endpoint();
			return endpoint.address().to_string();
		}

	private:
		void ReadHeader();
		void ReadBody();
		void WriteHeader();
		void WriteBody();

		void ProcessReceivedMessage(Message& message);

		asio::io_context& _io_context;
		asio::ip::tcp::socket _socket;

		std::unordered_map<uint8_t, std::shared_ptr<CategorizedConnectionHandler>> _connectionTypeQueues;
		std::queue<Message> _outgoingMessages;

		//buffers
		Message _readBuffer{};
	};
}
