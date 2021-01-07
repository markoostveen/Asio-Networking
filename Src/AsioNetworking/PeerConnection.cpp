#include "PeerConnection.h"
#include "CategorizedConnectionHandler.h"
#include "Server.h"

#include <memory>
#include <iostream>
#include <atomic>

namespace Networking {

	std::atomic<uint32_t> PeerId;

	PeerConnection::PeerConnection(Server* server, asio::io_context& context, asio::ip::tcp::socket socket)
		: _server(server), _io_context(context), _socket(std::move(socket)), _id(PeerId++) { // initialize id with increment of atomic value to ensure it's unique
		ReadHeader(); // start polling for new messages
	}

	void PeerConnection::SendMessageToPeer(Message message)
	{
		asio::post(_io_context,
			[this, message]()
			{
				bool bWritingMessage = !_outgoingMessages.empty();
				_outgoingMessages.push(message);
				if (!bWritingMessage) {
					WriteHeader();
				}
			});
	}

	void PeerConnection::ReadHeader()
	{
		asio::async_read(_socket, asio::buffer(&_readBuffer.Header, sizeof(MessageHeader)),
			[this](std::error_code errorCode, std::size_t length)
			{
				if (!errorCode)
				{
					if (_readBuffer.Header.Size > 0)
					{
						// allocate space for the message Body
						_readBuffer.Body = new uint8_t[_readBuffer.Header.Size];
						ReadBody();
					}
					else
					{
						_readBuffer.Body = nullptr;
						ProcessReceivedMessage(_readBuffer);
					}
				}
				else
				{
					std::cout << errorCode.message() << std::endl;
					_socket.close();
				}
			});
	}

	void PeerConnection::ReadBody()
	{
		asio::async_read(_socket, asio::buffer(_readBuffer.Body, _readBuffer.Header.Size),
			[this](std::error_code errorCode, std::size_t length)
			{
				if (!errorCode)
				{
					ProcessReceivedMessage(_readBuffer);
				}
				else
				{
					std::cout << errorCode.message() << std::endl;
					_socket.close();
				}
			});
	}

	void PeerConnection::WriteHeader()
	{
		asio::async_write(_socket, asio::buffer(&_outgoingMessages.front().Header, sizeof(MessageHeader)),
			[this](std::error_code errorCode, std::size_t length)
			{
				if (!errorCode)
				{
					if (_outgoingMessages.front().Header.Size > 0)
					{
						WriteBody();
					}
					else
					{
						_outgoingMessages.pop();

						if (!_outgoingMessages.empty())
						{
							WriteHeader();
						}
					}
				}
				else
				{
					std::cout << errorCode.message() << std::endl;
					_socket.close();
				}
			});
	}

	void PeerConnection::WriteBody()
	{
		asio::async_write(_socket, asio::buffer(_outgoingMessages.front().Body, _outgoingMessages.front().Header.Size),
			[this](std::error_code errorCode, std::size_t length)
			{
				if (!errorCode)
				{
					_outgoingMessages.pop();

					if (!_outgoingMessages.empty())
					{
						WriteHeader();
					}
				}
				else
				{
					std::cout << errorCode.message() << std::endl;
					_socket.close();
				}
			});
	}

	void PeerConnection::ProcessReceivedMessage(Message& message)
	{
		auto handler = _server->GetCategoryHandler(message.Header.Category);
		if (handler)
			handler->ProcessMessage(this, message);

		message.Reset();
		ReadHeader();
	}
}