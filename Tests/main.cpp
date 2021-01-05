#include "AsioNetworking/Server.h"

#include <memory>
#include <iostream>

using namespace Networking;

class TestServer : public Server {
public:
	TestServer(asio::io_context& io_context, short port) : Server(io_context, port) {}

protected:
	bool OnPeerConnected(PeerConnection& newPeer) final {
		// User should add their own handlers
		//newPeer.AddCategoryCallback(Id, handlerptr);

		return true;
	}
};

int main(int argc, char** argv) {
	asio::io_context io_context;
	std::shared_ptr<TestServer> server = std::make_shared<TestServer>(io_context, 4999);

	if (argc == 1)
		server->Connect("PiServer2", 5000);

	server->WaitForIncomingConnection();

	while (true) {
		io_context.poll(); // execute stuff for the server
	}

	return 0;
}