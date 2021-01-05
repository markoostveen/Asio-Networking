#include "AsioNetworking/Server.h"

#include <memory>
#include <iostream>
#include <thread>

using namespace Networking;

class TestServer : public Server {
public:
	TestServer(asio::io_context& io_context, short port) : Server(io_context, port) {}

protected:
	bool OnPeerConnected(PeerConnection* newPeer) final {
		// User should add their own handlers
		//newPeer.AddCategoryCallback(Id, handlerptr);

		return true;
	}
};

int main(int argc, char* argv[]) {
	asio::io_context io_context;

	std::shared_ptr<TestServer> server;
	if (argc == 1)
	{
		server = std::make_shared<TestServer>(io_context, 7500);
	}
	else {
		std::string portString = argv[1];
		server = std::make_shared<TestServer>(io_context, std::stoi(portString));
	}

	if (argc == 3) {
		server->Connect("127.0.0.1", std::stoi(argv[2]));
	}
	else {
		//server->Connect("PiServer2", 5000);
	}

	server->WaitForIncomingConnection();

	//std::thread worker([&]() {io_context.run(); }); // execute stuff for the server

	while (true) {
		io_context.run();
	}

	return 0;
}