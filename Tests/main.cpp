#include "AsioNetworking/server.h"

#include <memory>
#include <iostream>

using namespace Networking;

class TestServer : public Server {
public:
	TestServer(asio::io_context& io_context, short port) : Server(io_context, port) {}

protected:
	bool OnPeerConnected(PeerConnection& newPeer) final {
		// User should add their own handlers

		return true;
	}
};

asio::io_service io_service;
int main(int argc, char** argv) {
	asio::io_service io_service;
	std::shared_ptr<TestServer> server = std::make_shared<TestServer>(io_service, 22);

	if (argc == 1)
		server->Connect("127.0.0.1", 5000);

	server->WaitForIncomingConnection();

	while (true) {
		io_service.poll(); // execute stuff for the server
	}

	return 0;
}