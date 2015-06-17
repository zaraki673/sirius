/*
Derived from main.cpp in DjiNN
*/

#include "SennaService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include "tonic.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <glog/logging.h>

#include "boost/program_options.hpp"

#include "SENNA_utils.h"
#include "SENNA_Hash.h"
#include "SENNA_Tokenizer.h"
#include "SENNA_POS.h"
#include "SENNA_CHK.h"
#include "SENNA_NER.h"

#include "socket.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

//SennaServer.cpp

class SennaServiceHandler : virtual public SennaServiceIf {
 public:
  SennaServiceHandler() {
       // Let the server take care of initialization
      opt_path = NULL;
      opt_usrtokens = 0;

       /* the real thing */
      //char target_vb[MAX_TARGET_VB_SIZE];
      chk_labels = NULL;
      pt0_labels = NULL;
      pos_labels = NULL;
      ner_labels = NULL;

      /* inputs */
      word_hash = SENNA_Hash_new(opt_path, "hash/words.lst");
      caps_hash = SENNA_Hash_new(opt_path, "hash/caps.lst");
      suff_hash = SENNA_Hash_new(opt_path, "hash/suffix.lst");
      gazt_hash = SENNA_Hash_new(opt_path, "hash/gazetteer.lst");

      gazl_hash = SENNA_Hash_new_with_admissible_keys(
        opt_path, "hash/ner.loc.lst", "data/ner.loc.dat");
      gazm_hash = SENNA_Hash_new_with_admissible_keys(
        opt_path, "hash/ner.msc.lst", "data/ner.msc.dat");
      gazo_hash = SENNA_Hash_new_with_admissible_keys(
        opt_path, "hash/ner.org.lst", "data/ner.org.dat");
      gazp_hash = SENNA_Hash_new_with_admissible_keys(
        opt_path, "hash/ner.per.lst", "data/ner.per.dat");

       /* labels */
      pos_hash = SENNA_Hash_new(opt_path, "hash/pos.lst");
      chk_hash = SENNA_Hash_new(opt_path, "hash/chk.lst");
      ner_hash = SENNA_Hash_new(opt_path, "hash/ner.lst");

      // weights not used
      pos = SENNA_POS_new(opt_path, "data/pos.dat");
      chk = SENNA_CHK_new(opt_path, "data/chk.dat");
      ner = SENNA_NER_new(opt_path, "data/ner.dat");

      /* tokenizer */
      tokenizer =
       SENNA_Tokenizer_new(word_hash, caps_hash, suff_hash, gazt_hash, gazl_hash,
                          gazm_hash, gazo_hash, gazp_hash, opt_usrtokens);

  }

  void senna_all(std::string& _return, const TonicInput& tInput) {
    // Your implementation goes here
     setTonicApp(tInput);

     if (app.task == "pos")
    	 app.pl.size = pos->window_size *
                  (pos->ll_word_size + pos->ll_caps_size + pos->ll_suff_size);
     else if (app.task == "chk") {
    	app.pl.size = chk->window_size *
                  (chk->ll_word_size + chk->ll_caps_size + chk->ll_posl_size);
     } else if (app.task == "ner") {
    	int input_size = ner->ll_word_size + ner->ll_caps_size + ner->ll_gazl_size +
                     ner->ll_gazm_size + ner->ll_gazo_size + ner->ll_gazp_size;
 	   app.pl.size = ner->window_size * input_size;
     }

     //modify before the line "read the input file"
    
        // read input file
    ifstream file(app.input.c_str());
    string str;
    string text;
    while (getline(file, str)) text += str;	
      
    //tokenize
    SENNA_Tokens *tokens = SENNA_Tokenizer_tokenize(tokenizer, text.c_str());
    app.pl.num = tokens->n;

    if (app.pl.num == 0) LOG(FATAL) << app.input << " empty or no tokens found.";



    for (int i = 0; i < tokens->n; i++) {
      printf("%15s", tokens->words[i]);
      if (app.task == "pos")
        printf("\t%10s", SENNA_Hash_key(pos_hash, pos_labels[i]));
      else if (app.task == "chk")
        printf("\t%10s", SENNA_Hash_key(chk_hash, chk_labels[i]));
      else if (app.task == "ner")
        printf("\t%10s", SENNA_Hash_key(ner_hash, ner_labels[i]));
      printf("\n");
    }
    //end of sentence

      printf("senna_all\n");
  }

 protected:
  TonicSuiteApp app;

  //initializes the TonicSuiteApp which just stores inputs
  //don't think we'll need to use this 
  void setTonicApp(const& TonicInput& tInput){
	app.task = tInput.task;
	app.network = tInput.network;
	app.weights = tInput.weights;
	app.input = tInput.input;
	app.gpu = tInput.gpu;
	app.djinn = tInput.djinn;
	app.hostname ="localhost";
	app.portno = 8080;
	//app.socketfd

  //initializes the DNN
	if (app.djinn) {
	  app.hostname = tInput.hostname;
    	  app.portno = tInput.portno;
    	  //app.socketfd = CLIENT_init(app.hostname.c_str(), app.portno, debug);
    	  //if (app.socketfd < 0) exit(0);
	} else {
   	   app.net = new Net<float>(app.network);
   	   app.net->CopyTrainedLayersFrom(app.weights);
   	   if (app.gpu)
     		 Caffe::set_mode(Caffe::GPU);
   	   else
     		 Caffe::set_mode(Caffe::CPU);
  	}
	strcpy(app.pl.req_name, app.task.c_str());
  }  

  //Member data

  //parameters
  char *opt_path;
  int opt_usrtokens;
  /* the real thing */
  //char target_vb[MAX_TARGET_VB_SIZE];
  int *chk_labels;
  int *pt0_labels;
  int *pos_labels;
  int *ner_labels;

  /* inputs */
  SENNA_Hash *word_hash;
  SENNA_Hash *caps_hash;
  SENNA_Hash *suff_hash;
  SENNA_Hash *gazt_hash;

  SENNA_Hash *gazl_hash;
  SENNA_Hash *gazm_hash;
  SENNA_Hash *gazo_hash;
  SENNA_Hash *gazp_hash;

  /* labels */
  SENNA_Hash *pos_hash;
  SENNA_Hash *chk_hash;
  SENNA_Hash *ner_hash;

  // weights not used
  SENNA_POS *pos;
  SENNA_CHK *chk;
  SENNA_NER *ner;

};

//function for pasing command line
po::variables_map parse_opts(int ac, char** av) {
  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "Produce help message")(
      "common,c", po::value<string>()->default_value("../common/"),
      "Directory with configs and weights")(
      "portno,p", po::value<int>()->default_value(8080),
      "Port to open DjiNN on")

      ("nets,n", po::value<string>()->default_value("nets.txt"),
       "File with list of network configs (.prototxt/line)")(
          "weights,w", po::value<string>()->default_value("weights/"),
          "Directory containing weights (in common)")

          ("gpu,g", po::value<bool>()->default_value(false), "Use GPU?")(
              "debug,v", po::value<bool>()->default_value(false),
              "Turn on all debug")("threadcnt,t",
                                   po::value<int>()->default_value(-1),
                                   "Number of threads to spawn before exiting "
                                   "the server. (-1 loop forever)");

  po::variables_map vm;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << desc << "\n";
    exit(1);
  }
  return vm;
}

int main(int argc, char **argv) {
  //parse command line for arguments
  po::variables_map vm = parse_opts(argc, argv);

  int port = 9090;
  shared_ptr<SennaServiceHandler> handler(new SennaServiceHandler());
  shared_ptr<TProcessor> processor(new SennaServiceProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}







