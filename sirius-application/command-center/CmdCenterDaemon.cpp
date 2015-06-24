#include "CmdCenterDaemon.h"

//---- Functions designed for multithreading ----//
void immWorker(void *arg);
void asrWorker(void *arg);

//---- Utilities ----//
std::string parseImgFile(const std::string& immRetVal);

class CommandCenterHandler : public CommandCenterIf
{
public:
	// ctor: initialize command center's tables
	CommandCenterHandler()
	{
		registeredServices = std::multimap<std::string, ServiceData*>();
		// registeredServices = std::multimap<std::string, MachineData>();
		//boost::thread *heartbeatThread = new boost::thread(boost::bind(&CommandCenterHandler::heartbeatManager, this));
		
	}

	// (dtor defined in CommandCenter.h)

	virtual void registerService(const std::string& serviceType, const MachineData& mDataObj)
	{
		cout << "/-----registerService()-----/" << endl;
		cout << "received request from " << mDataObj.name
		     << ":" << mDataObj.port << ", serviceType = " << serviceType
		     << endl;

		registeredServices.insert( std::pair<std::string, ServiceData*>(serviceType, new ServiceData(mDataObj.name, mDataObj.port)));
		
		// registeredServices.insert( std::pair<std::string, MachineData>(serviceType, mDataObj) );
	
		cout << "There are now " << registeredServices.size() << " registered services" << endl;
		cout << "LIST OF REGISTERED SERVICES:" << endl;
		std::multimap<std::string, ServiceData*>::iterator it;
		// std::multimap<std::string, MachineData>::iterator it;
		for (it = registeredServices.begin(); it != registeredServices.end(); ++it)
		{
			cout << "\t" << it->first << endl;
			     // << it->second->name << ":"
			     // << (*it).second.port << endl;
		}
	}

	virtual void handleRequest(std::string& _return, const QueryData& data)
	{
		cout << "/-----handleRequest()-----/" << endl;

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

		//---- Create clients ----//
		// ServiceData *sd = NULL;
		AsrServiceData *asr = NULL;
		ImmServiceData *imm = NULL;
		QaServiceData *qa = NULL;
		//---- Kaldi speech recognition client
		if (data.audioData != "") {
			cout << "Getting asr client...\t";
			try {
				asr = new AsrServiceData(assignService("ASR"));
			} catch (AssignmentFailedException exc) {
				cout << exc.err << endl;
				return;
			}
			// asr = new AsrServiceData(sd);
			cout << "AsrServiceData object constructed" << endl;
		}
		//---- Image matching client
		if (data.imgData != "") {
			cout << "Getting imm client...\t";
			try {
				imm = new ImmServiceData(assignService("IMM"));
			} catch (AssignmentFailedException exc) {
				cout << exc.err << endl;
				return;
			}
			// imm = new ImmServiceData(sd);
			cout << "ImmServiceData object constructed" << endl;
		}
		//---- Open Ephyra QA client
		// TODO For now, this is always generated, because the command center
		// cannot yet determine whether the audio data is a voice command
		// (which doesn't require QA) or a voice query (which does require QA).
		cout << "Getting qa client...\t";
		try {
			qa = new QaServiceData(assignService("QA"));
		} catch (AssignmentFailedException exc) {
			cout << exc.err << endl;
			return;
		}
		// qa = new QaServiceData(sd);
		cout << "QaServiceData object constructed" << endl;

		//---- Set audio and imm fields in service data objects
		// if they were constructed
		if (asr) {
			asr->audio = binary_audio;
		} if (imm) {
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
			cout << "Now trying the threading imm method" << endl;
			AsrWorker asrworker_obj;
			ImmWorker immworker_obj;
			boost::function<void()> asrfunc = boost::bind(&AsrWorker::execute, &asrworker_obj, (void *) asr);
			boost::function<void()> immfunc = boost::bind(&ImmWorker::execute, &immworker_obj, (void *) imm);
			boost::thread asrthread(asrfunc);
			boost::thread immthread(immfunc);
			asrthread.join();
			immthread.join();
			cout << "Boost thread success!" << endl;
			cout << "ASR == " << asrworker_obj.returnValue << endl;
			cout << "IMM == " << immworker_obj.returnValue << endl;

			question = asrworker_obj.returnValue + " " + immworker_obj.returnValue;
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
	std::multimap<std::string, ServiceData*> registeredServices;
	// std::multimap<std::string, MachineData> registeredServices;
	boost::thread heartbeatThread;

	ServiceData* assignService(const std::string type) {
		//load balancer for service assignment
		std::multimap<std::string, ServiceData*>::iterator it;
		it = registeredServices.find(type);
		if (it != registeredServices.end()) {
			//sd = new ServiceData(it->second.name, it->second.port);
			// cout << "Selected " << it->second.name << ":" << it->second.port
			//      << " for " << type << " server" << endl;
			return it->second;
		} else {
			string msg = type + " requested, but not found";
			cout << msg << endl;
			throw(AssignmentFailedException(type + " requested, but not found"));
			return NULL;
		}
	}

	void heartbeatManager(){
		// cout << "heartbeat manager started" << endl;
		// std::multimap<std::string, ServiceData*>::iterator it;
		// while(true) {
		// 	for(it = registeredServices.begin(); it != registeredServices.end(); ++it){
		// 		//TO DO: add locking 
		// 		std::string type = it->first;
		// 		try{
		// 			if(type == "ASR") {
		// 				AsrServiceData *asr = new AsrServiceData(it->second);
		// 				asr->transport->open();
		// 				asr->client.ping();
		// 				asr->transport->close();
		// 			} else if(type == "IMM") {
		// 				ImmServiceData *imm = new ImmServiceData(it->second);
		// 				imm->transport->open();
		// 				imm->client.ping();
		// 				imm->transport->close();
		// 			} else if(type == "QA") {
		// 				QaServiceData *qa = new QaServiceData(it->second);
		// 				qa->transport->open();
		// 				qa->client.ping();
		// 				qa->transport->close();
		// 			} else {
		// 				cout << "Found unknown type --" << type << "-- in registered services" << endl;
		// 			}
		// 		} catch(...) {
		// 			//remove from list
		// 			registeredServices.erase(it);
		// 			cout << "There are now " << registeredServices.size() << " registered services" << endl;
		// 			cout << "LIST OF REGISTERED SERVICES:" << endl;
		// 			std::multimap<std::string, ServiceData*>::iterator it;
		// 			// std::multimap<std::string, MachineData>::iterator it;
		// 			for (it = registeredServices.begin(); it != registeredServices.end(); ++it)
		// 			{
		// 				cout << "\t" << it->first << endl;
		// 				     // << it->second->name << ":"
		// 				     // << (*it).second.port << endl;
		// 			}
		// 			break;
		// 		}
		// 	}
		// 	//sleep
		// 	boost::posix_time::seconds sleepTime(60);
		// 	boost::this_thread::sleep(sleepTime);

		// }
		
		// cout << "heartbeat manager finished" << endl;
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

//---- Functions designed for multithreading ----//
void ImmWorker::execute(void *arg)
{
	ImmServiceData *imm = (ImmServiceData *) arg;
	imm->transport->open();
	imm->client.match_img(returnValue, imm->img);
	imm->transport->close();
	cout << "imm worker thread... IMG = " << returnValue << endl;
	// TODO: EXCEPTION HANDLING WITH BOOST
	// image filename parsing
	returnValue = parseImgFile(returnValue);
}

void AsrWorker::execute(void *arg)
{
	AsrServiceData *asr = (AsrServiceData *) arg;
	asr->transport->open();
	asr->client.kaldi_asr(returnValue, asr->audio);
	asr->transport->close();
	cout << "asr worker thread... ASR = " << returnValue << endl;
}
