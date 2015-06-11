#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>


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
	boost::shared_ptr<TTransport> socket(new TSocket("localhost", port));
	boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	CommandCenterClient client(protocol);

	try
	{
		//std::string question(argv[1]);
		std::string answer;

		//Audio Files
		//ifstream fin_audio("test.wav",ios::binary);
		ifstream fin_audio("what.is.the.capital.of.italy.wav",ios::binary);
		ostringstream ostrm_audio;
		ostrm_audio <<fin_audio.rdbuf();
		std::string capital_italy_audio_file(ostrm_audio.str());
		
		ifstream fin_audio01("how.tall.is.this.wav",ios::binary);
		ostringstream ostrm_audio01;
		ostrm_audio01 <<fin_audio01.rdbuf();
		std::string how_tall_audio_file(ostrm_audio01.str());	
	
		//Image File
		ifstream fin_image("test.jpg", ios::binary);
		ostringstream ostrm_image;
		ostrm_image << fin_image.rdbuf();
		string image_file(ostrm_image.str());
		// TODO: this initialization is stupid, but thrift doesn't
		// generate more helpful ctors
		QueryType qTypeObj; //TODO
		qTypeObj.ASR = true;
		qTypeObj.QA = false;
		qTypeObj.IMM = false;

		QueryType asrqa_qTypeObj;
		asrqa_qTypeObj.ASR = true;
		asrqa_qTypeObj.QA = true;
		asrqa_qTypeObj.IMM = false;

		QueryType asrqaimm_qTypeObj;
		asrqaimm_qTypeObj.ASR = true;
		asrqaimm_qTypeObj.QA = true;
		asrqaimm_qTypeObj.IMM = true;

		File audioFile00;
		audioFile00.file = capital_italy_audio_file;
		audioFile00.b64format = false;

		File audioFile01;
		audioFile01.file = how_tall_audio_file;
		audioFile01.b64format = false;

		File textFile;
		textFile.file = "what is the speed of light?";
		textFile.b64format = false;

		File imgFile;
		imgFile.file = image_file;
		imgFile.b64format = false;

		QueryData capital_italy;
		capital_italy.audioFile = audioFile00;
		capital_italy.textFile = textFile;
		capital_italy.imgFile = imgFile;

		QueryData how_tall;
		how_tall.audioFile = audioFile01;
		how_tall.textFile = textFile;
		how_tall.imgFile = imgFile;

		//DEBUG
		/*QueryData capital_italy;
		capital_italy.audioFile = capital_italy_audio_file;
		capital_italy.textFile = "";
		capital_italy.imgFile = "";*/

		transport->open();
		//client.askTextQuestion(answer, question);


		cout << "///// ASR /////" << endl;
		client.handleRequest(answer, qTypeObj, capital_italy);
		cout << "ANSWER = " << answer << endl;

		cout << "///// ASR-QA /////" << endl;
		client.handleRequest(answer, asrqa_qTypeObj, capital_italy);
		cout << "ANSWER = " << answer << endl;

		cout << "\n///// ASR-QA-IMM /////" << endl;
		client.handleRequest(answer, asrqaimm_qTypeObj, how_tall);
		cout << "ANSWER = " << answer << endl;

		transport->close();
		
	} catch(TException &tx) {
                cout << "CLIENT ERROR: " << tx.what() << endl;
        }
	return 0;
}
