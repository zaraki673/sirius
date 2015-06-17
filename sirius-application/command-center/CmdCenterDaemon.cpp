#include "CmdCenterDaemon.h"

//---- Functions designed for multithreading with pthreads ----//
// NOTE: These functions CANNOT be declared as members of a class
// when using pthreads. See stackoverflow, cannot-convert-voidmyclassvoid-to...
void *immWorker(void *arg);
void *asrWorker(void *arg);

//---- Utilities ----//
std::string parseImgFile(const std::string& immRetVal);

class CommandCenterHandler : public CommandCenterIf
{
public:
	// ctor: initialize command center's tables
	CommandCenterHandler()
	{
		registeredServices = std::multimap<std::string, MachineData>();
		boost::thread *heartbeatThread = new boost::thread(boost::bind(&CommandCenterHandler::heartbeatManager, this));
		
	}

	// (dtor defined in CommandCenter.h)

	virtual void registerService(const std::string& serviceType, const MachineData& mDataObj)
	{
		cout << "/-----registerService()-----/" << endl;
		cout << "received request from " << mDataObj.name
		     << ":" << mDataObj.port << ", serviceType = " << serviceType
		     << endl;
		registeredServices.insert( std::pair<std::string, MachineData>(serviceType, mDataObj) );
	
		cout << "There are now " << registeredServices.size() << " registered services" << endl;
		cout << "LIST OF REGISTERED SERVICES:" << endl;
		std::multimap<std::string, MachineData>::iterator it;
		for (it = registeredServices.begin(); it != registeredServices.end(); ++it)
		{
			cout << "\t" << (*it).first << "\t"
			     << (*it).second.name << ":"
			     << (*it).second.port << endl;
		}
	}

	virtual void handleRequest(std::string& _return, const QueryData& data)
	{
		cout << "/-----handleRequest()-----/" << endl;

		//---- Create clients ----//
		ServiceData *sd = NULL;
		AsrServiceData *asr = NULL;
		ImmServiceData *imm = NULL;
		QaServiceData *qa = NULL;
		//---- Kaldi speech recognition client
		if (data.audioData != "") {
			cout << "Getting asr client...\t";
			try {
				assignService(sd, "ASR");
			} catch (AssignmentFailedException exc) {
				cout << exc.err << endl;
				return;
			}
			asr = new AsrServiceData(sd);
			cout << "AsrServiceData object constructed" << endl;
		}
		//---- Image matching client
		if (data.imgData != "") {
			cout << "Getting imm client...\t";
			try {
				assignService(sd, "IMM");
			} catch (AssignmentFailedException exc) {
				cout << exc.err << endl;
				return;
			}
			imm = new ImmServiceData(sd);
			cout << "ImmServiceData object constructed" << endl;
		}
		//---- Open Ephyra QA client
		// TODO For now, this is always generated, because the command center
		// cannot yet determine whether the audio data is a voice command
		// (which doesn't require QA) or a voice query (which does require QA).
		cout << "Getting qa client...\t";
		try {
			assignService(sd, "QA");
		} catch (AssignmentFailedException exc) {
			cout << exc.err << endl;
			return;
		}
		qa = new QaServiceData(sd);
		cout << "QaServiceData object constructed" << endl;

		//---- Transform data into a form the services can use ----//
		std::string binary_audio, binary_img;
		if(data.audioB64Encoding) {
			cout << "Decoding audio..." << endl;
			binary_audio = base64_decode(data.audioData);	
		} else {
			binary_audio = data.audioData;
		}
		if(data.imgB64Encoding) {
			cout << "Decoding img..." << endl;
			binary_img = base64_decode(data.imgData);
		} else {
			binary_img = data.imgData;
		}

		//---- Set audio and imm fields in service data objects
		// if they were constructed
		if (asr)
		{
			asr->audio = binary_audio;
		}
		if (imm)
		{
			imm->img = binary_img;
		}

		//---- Run pipeline ----//
		std::string asrRetVal = "";
		std::string immRetVal = "";
		std::string question = "";
		// TODO: use nlp libs to distinguish between voice cmd
		// and voice query
		if ((data.audioData != "") && (data.imgData != ""))
		{
//			cout << "Starting ASR-IMM-QA pipeline..." << endl;
			//---- Run asr and image matching in parallel
//			//---Image matching
//			imm->transport->open();
//			imm->client.match_img(immRetVal, binary_img);
//			imm->transport->close();
//			cout << "IMG = " << immRetVal << endl;
//			// image filename parsing
//			try
//			{
//				immRetVal = parseImgFile(immRetVal);
//			}
//			catch (BadImgFileException)
//			{
//				cout << "Cmd Center: BadImgFileException" << endl;
//				throw;
//			}

			///////////////////////////////////////////////////////////
			// NOTE: declaring a void **s and passing that to pthread_join
			// doesn't work.
			cout << "Now trying the threading imm method" << endl;
			pthread_t thr[2];
			int rc;
			void *asrstatus = NULL;
			void *immstatus = NULL;
			if ((rc = pthread_create(&thr[0], NULL, asrWorker, (void *) asr)))
			{
				cerr << "error: pthread_create: " << rc << endl;
			}
			if ((rc = pthread_create(&thr[1], NULL, immWorker, (void *) imm)))
			{
				cerr << "error: pthread_create: " << rc << endl;
			}
			pthread_join(thr[0], &asrstatus);
			pthread_join(thr[1], &immstatus);
			assert(immstatus && asrstatus);
			cout << "SUCCESS!" << endl;

			///////////////////////////////////////////////////////////

//			//---Speech recognition
//			asr->transport->open();
//			asr->client.kaldi_asr(asrRetVal, binary_audio);
//			asr->transport->close();
			
			//---Question answer
			ResponseData *asrresp = (ResponseData *) asrstatus;
			ResponseData *immresp = (ResponseData *) immstatus;
			assert(asrresp && immresp);
			question = asrresp->getResponse() + " " + immresp->getResponse();
			cout << "Your new question is: " << question << endl;
			qa->transport->open();
			qa->client.askFactoidThrift(_return, question);
			qa->transport->close();
		}
		else if (data.audioData != "")
		{
			cout << "Starting ASR-QA pipeline..." << endl;
			asr->transport->open();
			asr->client.kaldi_asr(asrRetVal, binary_audio);
			asr->transport->close();
	
			qa->transport->open();
			qa->client.askFactoidThrift(_return, asrRetVal);
			qa->transport->close();
		}
		/*else if (data.audioData != "")
		{
			asr_transport->open();
			asr_client.kaldi_asr(_return, binary_audio);
			asr_transport->close();
		}*/
		else if (data.textData != "")
		{
			qa->transport->open();
			qa->client.askFactoidThrift(_return, data.textData);
			qa->transport->close();
		}
		else if (data.imgData != "")
		{
			imm->transport->open();
			imm->client.match_img(_return, binary_img);
			imm->transport->close();
		}
		else
		{
			cout << "Nothing in the pipeline" << endl;
		}

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
	// TODO: this is a poor model, because it doesn't allow you to
	// select available servers easily, for a given key.
	std::multimap<std::string, MachineData> registeredServices;
	// boost::thread heartbeatThread;

	void assignService(ServiceData *&sd, const std::string type) {
		//load balancer for service assignment
		std::multimap<std::string, MachineData>::iterator it;
		it = registeredServices.find(type);
		if (it != registeredServices.end()) {
			sd = new ServiceData(it->second.name, it->second.port);
			cout << "Selected " << it->second.name << ":" << it->second.port
			     << " for " << type << " server" << endl;
		} else {
			string msg = type + " requested, but not found";
			cout << msg << endl;
			throw(AssignmentFailedException(type + " requested, but not found"));
		}
	}

	void heartbeatManager(){
		cout << "heartbeat manager started" << endl;
		// boost::posix_time::seconds workTime(3);
		// boost::this_thread::sleep(workTime);
		cout << "heartbeat manager finished" << endl;
	}

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
	/*TMultiplexedProcessor multiplexed_processor = new TMultiplexedProcessor();
	multiplexed_processor.registerProcessor(
		"cmdcenter",
		processor
	);*/

	boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

	// Initialize the command center server.
	// The server listens for packets transmitted via <transport> in <protocol> format.
	// The processor handles serialization/deserialization and communication w/ handler.
	//TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
	//TSimpleServer server(multiplexed_processor, serverTransport, transportFactory, protocolFactory);

	// initialize the thread manager and factory
	boost::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(THREAD_WORKS);
	boost::shared_ptr<PosixThreadFactory> threadFactory = boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
	threadManager->threadFactory(threadFactory);
	threadManager->start();

	// initialize the image matching server
	TThreadPoolServer server(processor, serverTransport, transportFactory, protocolFactory, threadManager);
	cout << "Starting the command center server on port " << port << "..." << endl;
	server.serve();
	cout << "Done." << endl;
	return 0;
}

//---- Functions designed for multithreading with pthreads ----//
void *immWorker(void *arg)
{
	std::string immRetVal = "";
	ImmServiceData *imm = (ImmServiceData *) arg;
	imm->transport->open();
	imm->client.match_img(immRetVal, imm->img);
	imm->transport->close();
	cout << "imm worker thread... IMG = " << immRetVal << endl;
	// image filename parsing
	try
	{
		immRetVal = parseImgFile(immRetVal);
	}
	catch (BadImgFileException)
	{
		cout << "Cmd Center: BadImgFileException" << endl;
		pthread_exit(NULL);
	}

	// Package response
	ResponseData *resp = new ResponseData(immRetVal);
	void *ret = (void *) resp;
	// NOTE: there doesn't appear to be functional difference between
	// return() and pthread_exit() in this case.
	//return ret;
	pthread_exit(ret);
}

void *asrWorker(void *arg)
{
	std::string asrRetVal = "";
	AsrServiceData *asr = (AsrServiceData *) arg;
	asr->transport->open();
	asr->client.kaldi_asr(asrRetVal, asr->audio);
	asr->transport->close();
	cout << "asr worker thread... ASR = " << asrRetVal << endl;

	ResponseData *resp = new ResponseData(asrRetVal);
	void *ret = (void *) resp;
	pthread_exit(ret);
}

std::string parseImgFile(const std::string& immRetVal)
{
	// Everything must be escaped twice
	const char *regexPattern = "\\A(/?)([\\w\\-]+/)*([\\w\\-]+)(\\.jpg)\\z";
	std::cout << "Passing the following pattern to regex engine: "
		<< regexPattern << std::endl;
	boost::regex re(regexPattern);
	std::string fmt("$3");
	std::string outstr = immRetVal;
	if (boost::regex_match(immRetVal, re))
	{
		std::cout << immRetVal << " matches pattern"  << std::endl;
		outstr = boost::regex_replace(immRetVal, re, fmt);
		std::cout << "Input: " << immRetVal << std::endl;
		std::cout << "Result: " << outstr << std::endl;
	}
	else
	{
		std::cout << "No match for " << immRetVal << "..." << std::endl;
		throw(BadImgFileException());
	}

	return outstr;
}


