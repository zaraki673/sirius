#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

// Thrift-generated stubs for RPC handling
#include "gen-cpp/CommandCenter.h"

using namespace std; 
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace cmdcenterstubs;

int main(int argc, char **argv) {
	int port = 8081;
	boost::shared_ptr<TTransport> socket(new TSocket("clarity04.eecs.umich.edu", port));
	boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	CommandCenterClient client(protocol);

	try
	{
		//std::string question(argv[1]);
		std::string answer;
		// TODO: this initialization is stupid, but thrift doesn't
		// generate more helpful ctors
		QueryType qTypeObj;
		qTypeObj.ASR = false;
		qTypeObj.QA = true;
		qTypeObj.IMM = false;

		QueryData data;
		data.audioFile = "";
		data.textFile = "what is the speed of light?";
		data.imgFile = "";

		transport->open();
	
		//client.askTextQuestion(answer, question);
		
		client.handleRequest(answer, qTypeObj, data);
	
		transport->close();
	} catch(TException &tx) {
                cout << "CLIENT ERROR: " << tx.what() << endl;
        }
	return 0;
}
