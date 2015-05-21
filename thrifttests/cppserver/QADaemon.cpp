// prepend a bunch of thrift source code to this document (preprocessor)
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <string>

// prepend all thrift generated code to this document as well
// this generated cpp code handles serialization and transport
// of data from/to cpp processes
#include "gen-cpp/QAService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

// The handler must be a child of the QA service interface generated
// by thrift
class QAServiceHandler : public QAServiceIf
{
	public:
		// calls default ctors of the bases and the non-static members of this class
		QAServiceHandler() {}
		void AskQuestion(const std::vector<std::string> & question) {
			cout << "Your question was: " << question.at(0) << endl;
			cout << "Your answer is: 42" << endl;
		}
};

int main()
{

	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	boost::shared_ptr<QAServiceHandler> handler(new QAServiceHandler());
	boost::shared_ptr<TProcessor> processor(new QAServiceProcessor(handler));
	boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(9090));
	boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

	TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

	cout << "Starting the server..." << endl;
	server.serve();
	cout << "Done." << endl;
	return 0;
}
