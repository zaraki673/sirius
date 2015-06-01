#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <thrift/protocol/TBinaryProtocol.h>             
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TSocket.h>                    
#include <thrift/transport/TBufferTransports.h>          
#include <thrift/transport/TTransportUtils.h>

#include "KaldiService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;



int main(int argc, char ** argv){

	boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
	boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	KaldiServiceClient client(protocol);

	transport->open();
	string answer;
	string audio_file= argv[1];
	
	ifstream fin(audio_file.c_str(), ios::binary);

if (!fin) std::cerr << "Could not open the file!" << std::endl;
	ostringstream ostrm;
  ostrm << fin.rdbuf();
	string audio_file_to_send(ostrm.str());

	client.kaldi_asr(answer, audio_file_to_send);

	cout<< answer<< endl;

	transport->close();
	return 0;

}
