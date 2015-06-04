// C++ thrift headers 
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>

// Useful C++ headers
#include <iostream>
#include <stdexcept>
#include <sstream>

// Thrift-generated stubs for RPC handling
#include "gen-cpp/CommandCenter.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace cmdcenterstubs;

class CommandCenterHandler : public CommandCenterIf
{
public:
	// ctor: initialize command center's tables
	CommandCenterHandler() {}

	// (dtor defined in CommandCenter.h)

	virtual void registerService(const std::string& machineName, const int32_t port, const std::string& serviceType)
	{
		cout << "received request from " << machineName << ":" << port << ", serviceType = " << serviceType << endl;
	}

	virtual void askTextQuestion(std::string& _return, const std::string& question) {}

private:
	// command center's tables
	
};

int main() {
	int port = 8080;
	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	boost::shared_ptr<CommandCenterHandler> handler(new CommandCenterHandler());
	boost::shared_ptr<TProcessor> processor(new CommandCenterProcessor(handler));
	boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

	// Initialize the command center server.
	// The server listens for packets transmitted via <transport> in <protocol> format.
	// The processor handles serialization/deserialization and communication w/ handler.
	TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

	cout << "Starting the command center server on port " << port << "..." << endl;
	server.serve();
	cout << "Done." << endl;
	return 0;
}
