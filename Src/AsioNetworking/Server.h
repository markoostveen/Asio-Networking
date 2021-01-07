#pragma once

#include "config.h"

#include "PeerConnection.h"
#include "CategorizedConnectionHandler.h"

#include "asio.hpp"

#include <vector>
#include <string>
#include <iostream>

namespace Networking {
	class Server {
	public:
		Server(const Server&) = delete;
		Server() = delete;

		template<class T>
		void AddCategoryHandler(std::shared_ptr<T> categoryHandler) {
			_categoryHandlers.emplace(T::ID(), categoryHandler);
		}

		void RemoveCategoryHandler(uint8_t categoryId);

		template<class T>
		T* GetCategoryHandler() {
			return static_cast<T*>(GetCategoryHandler(T::ID()));
		}

		CategorizedConnectionHandlerBase* GetCategoryHandler(uint8_t categoryId);

		void WaitForIncomingConnection();
		bool Connect(const std::string& host, const int port);

		uint32_t ConnectedPeerCount();

		PeerConnection* GetPeer(uint32_t id);
		uint32_t GetPeerId(uint32_t index);

		short Port();

		void RefreshConnectedPeers();

		void Disconnect(const PeerConnection* peer);

	protected:
		Server(asio::io_context& io_context, short port);

		virtual bool OnPeerConnected(PeerConnection* newPeer) = 0;

	private:

		PeerConnection* AddPeer(asio::ip::tcp::socket socket);
		asio::io_context& _io_context;
		asio::ip::tcp::acceptor _acceptor;
		std::vector<std::unique_ptr<PeerConnection>> _peers;
		std::unordered_map<uint8_t, std::shared_ptr<CategorizedConnectionHandlerBase>> _categoryHandlers;
	};
}