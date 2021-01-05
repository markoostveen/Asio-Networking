#include "PeerConnection.h"
#include "CategorizedConnectionHandler.h"

#include <memory>
#include <iostream>

namespace Networking {
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

	void PeerConnection::AddCategoryCallback(uint8_t categoryId, std::shared_ptr<CategorizedConnectionHandler> categoryHandler)
	{
		_connectionTypeQueues.emplace(categoryId, categoryHandler);
	}

	void PeerConnection::RemoveCategoryCallback(uint8_t categoryId)
	{
		_connectionTypeQueues.erase(categoryId);
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
					std::cout << errorCode.message() << "\n";
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
					std::cout << errorCode.message() << "\n";
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
					std::cout << errorCode.message() << "\n";
					_socket.close();
				}
			});
	}

	void PeerConnection::ProcessReceivedMessage(Message& message)
	{
		auto it = _connectionTypeQueues.find(message.Header.Category);
		if (it != _connectionTypeQueues.end()) {
			it->second->ProcessMessage(this, message); // process the message using the callback
		}

		message.Reset();
		ReadHeader();
	}
}