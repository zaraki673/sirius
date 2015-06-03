

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

void clientAskQuestion(qastubs::QAServiceClient& client, std::vector<std::string>& qvec);
void clientAskFactoid(qastubs::QAServiceClient& client, std::string question);
void clientAskList(qastubs::QAServiceClient& client, std::string question);

int main(int argc, char** argv)
{
        boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
        boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        qastubs::QAServiceClient client(protocol);
	
	//struct timeval tv1, tv2;
	try {
		// Extract question from input
		std::string question(argv[1]);
		std::vector<std::string> qvec;
		qvec.push_back(question);

                transport->open();

		// ask question
		//clientAskQuestion(client, qvec);

		// ask factoid question
		clientAskFactoid(client, question);

		// ask list question
		//clientAskList(client, question);

                transport->close();
        } catch(TException &tx) {
                cout << "ERROR: " << tx.what() << endl;
        }

        return 0;
}

// qvec could be modified, since it isn't const
void clientAskQuestion(qastubs::QAServiceClient& client, std::vector<std::string>& qvec)
{
	struct timeval tv1, tv2;

	cout << "calling askQuestion():" << endl;
	gettimeofday(&tv1, NULL);
	client.askQuestion(qvec);
        gettimeofday(&tv2, NULL);
        unsigned int query_latency = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
        cout << "client sent the question successfully..." << endl;
        cout << "server replied within " << fixed << setprecision(2) << (double)query_latency / 1000 << " ms" << endl;
	cout << endl;		
}

void clientAskFactoid(qastubs::QAServiceClient& client, std::string question)
{
	struct timeval tv1, tv2;
	std::string answer;

	// ask factoid question
	cout << "calling askFactoidThrift():" << endl;
	gettimeofday(&tv1, NULL);
        client.askFactoidThrift(answer, question); // pass QAService a question
        gettimeofday(&tv2, NULL);
        unsigned int query_latency = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
        cout << "client sent the question successfully..." << endl;
        cout << "ANSWER = " << answer << endl;
        cout << "server replied within " << fixed << setprecision(2) << (double)query_latency / 1000 << " ms" << endl;
        // cout << fixed << setprecision(2) << (double)query_latency / 1000 << endl;
	cout << endl;

	// ask factoid debug question
	/*cout << "calling askFactoidThriftDebug():" << endl;
	gettimeofday(&tv1, NULL);
        client.askFactoidThriftDebug(answer, question); // pass QAService a question
        gettimeofday(&tv2, NULL);
        query_latency = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
        cout << "client sent the question successfully..." << endl;
        cout << "ANSWER = " << answer << endl;
        cout << "server replied within " << fixed << setprecision(2) << (double)query_latency / 1000 << " ms" << endl;
	cout << endl;
	*/
}

void clientAskList(qastubs::QAServiceClient& client, std::string question)
{
	struct timeval tv1, tv2;
	std::vector<std::string> answers;

	cout << "calling askListThrift():" << endl;
	gettimeofday(&tv1, NULL);
        client.askListThrift(answers, question); // pass QAService a question
        gettimeofday(&tv2, NULL);
        unsigned int query_latency = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
        cout << "client sent the question successfully..." << endl;
        
	cout << "printing answers:" << endl;
	std::vector<std::string>::iterator it;
	for (it = answers.begin(); it != answers.end(); ++it)
	{
		cout << "\t" << *it << endl;
	}

	cout << "server replied within " << fixed << setprecision(2) << (double)query_latency / 1000 << " ms" << endl;
	cout << endl;
}

