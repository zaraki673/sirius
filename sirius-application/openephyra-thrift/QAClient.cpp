

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/QAService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

int main(int argc, char** argv)
{
        boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
        boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        qastubs::QAServiceClient client(protocol);
	
	struct timeval tv1, tv2;
	try {
		// Extract question from input
		std::string question(argv[1]);
		std::string answer;
		//std::vector<std::string> qvec;
		//qvec.push_back(question);

                transport->open();

		gettimeofday(&tv1, NULL);
                client.askFactoidThrift(answer, question); // pass QAService a question
                gettimeofday(&tv2, NULL);
                unsigned int query_latency = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);

                 cout << "client sent the question successfully..." << endl;
                cout << answer << endl;
                cout << "server replied within " << fixed << setprecision(2) << (double)query_latency / 1000 << " ms" << endl;
                // cout << fixed << setprecision(2) << (double)query_latency / 1000 << endl;

                transport->close();
        } catch(TException &tx) {
                cout << "ERROR: " << tx.what() << endl;
        }

        return 0;
}
