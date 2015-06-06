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

// Additional C++ headers for querying registered services
#include <thrift/transport/TSocket.h>

// Useful C++ headers
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstdlib>

// Thrift-generated stubs for RPC handling
#include "gen-cpp/CommandCenter.h"

// Thrift-generated stubs for communicating with registered
// services
#include "../question-answer/openephyra-thrift/gen-cpp/QAService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace cmdcenterstubs;
using namespace qastubs;

class MachineData
{
public:
	MachineData(const std::string& _machineName, const int32_t _port)
		: machineName(_machineName), port(_port) {}
	std::string getMachineName() { return machineName; }
	int32_t getPort() { return port; }
private:
	std::string machineName;
	int32_t port;
};

class CommandCenterHandler : public CommandCenterIf
{
public:
	// ctor: initialize command center's tables
	CommandCenterHandler()
	{
		registeredServices = std::multimap<std::string, MachineData>();
	}

	// (dtor defined in CommandCenter.h)

	virtual void registerService(const std::string& machineName, const int32_t port, const std::string& serviceType)
	{
		cout << "received request from " << machineName
		     << ":" << port << ", serviceType = " << serviceType
		     << endl;
		MachineData mDataObj(machineName, port);
		registeredServices.insert( std::pair<std::string, MachineData>(serviceType, mDataObj) );
	
		// DEBUG information (testing only)
		cout << "There are now " << registeredServices.size() << " registered services" << endl;
		cout << "LIST OF REGISTERED SERVICES:" << endl;
		std::multimap<std::string, MachineData>::iterator it;
		for (it = registeredServices.begin(); it != registeredServices.end(); ++it)
		{
			cout << "\t" << (*it).first << "\t"
			     << (*it).second.getMachineName() << ":"
			     << (*it).second.getPort() << endl;
		}
		// END_DEBUG

	}

	virtual void askTextQuestion(std::string& _return, const std::string& question)
	{
		cout << "Command Center: askTextQuestion()" << endl;
		// TODO: this is hard-coded; make this extensible
		int serverPort = 9091;
		boost::shared_ptr<TTransport> socket(new TSocket("localhost", serverPort));
		boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		QAServiceClient client(protocol);
		try
		{
			// Extract question from input
			std::string answer;

			transport->open();
			
			// Ask question
			client.askFactoidThrift(answer, question);
			
			// Report response
			cout << "Command Center forwarded the question successfully..." << endl;
			cout << "ANSWER = " << answer << endl;
			cout << endl;
			transport->close();
		}
		catch (TException &tx)
		{
			cout << "COMMAND CENTER ERROR: " << tx.what() << endl;
		}

	}

	void ping()
	{
		cout << "ping!" << endl;
	}

private:
	// registeredServices: a table of all servers that registered with
	// the command center via the registerService() method
	// NOTE: eventually we'll replace this with a better structure
	std::multimap<std::string, MachineData> registeredServices;
	
};

int main(int argc, char **argv) {
	int port = 8081;
	if (argv[1])
	{
		port = atoi(argv[1]);
	}
	else
	{
		cout << "Command center port not specified; using default" << endl;
	}

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
