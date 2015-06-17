nclude "SennaService.h"
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
       // Your initialization goes here
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
	
     printf("senna_all\n");
  }

 protected:
  TonicSuiteApp app;

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

int main(int argc, char **argv) {
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







