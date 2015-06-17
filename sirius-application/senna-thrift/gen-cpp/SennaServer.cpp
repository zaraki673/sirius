/*
Derived from main.cpp in DjiNN
*/
//thread.cpp
#include <sstream>
#include <assert.h>
#include <stdint.h>
#include <ctime>
#include <cmath>
#include <boost/chrono/thread_clock.hpp>
#include "timer.h"

extern map<string, Net<float>*> nets;
extern bool debug;
extern bool gpu;

//thread.h
#include <vector>
#include "caffe/caffe.hpp"

using caffe::Blob;
using caffe::Caffe;
using caffe::Net;
using caffe::Layer;
using caffe::shared_ptr;
using caffe::Timer;
using caffe::vector;

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
	
     printf("senna_all\n");
  }

 protected:
  TonicSuiteApp app;

  //initializes the TonicSuiteApp which just stores inputs
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

  //handler request
  //helper function
  void SERVICE_fwd(float* in, int in_size, float* out, int out_size,
                 Net<float>* net) {
	string net_name = net->name();
  	STATS_INIT("service", "DjiNN service inference");
  	PRINT_STAT_STRING("network", net_name.c_str());

  	if (Caffe::mode() == Caffe::CPU)
   	 PRINT_STAT_STRING("platform", "cpu");
  	else
   	 PRINT_STAT_STRING("platform", "gpu");

  	float loss;
  	vector<Blob<float>*> in_blobs = net->input_blobs();

  	tic();
  	in_blobs[0]->set_cpu_data(in);
 	 vector<Blob<float>*> out_blobs = net->ForwardPrefilled(&loss);
  	memcpy(out, out_blobs[0]->cpu_data(), sizeof(float));

  	PRINT_STAT_DOUBLE("inference latency", toc());

  	STATS_END();

  	if (out_size != out_blobs[0]->count())
    		LOG(FATAL) << "out_size =! out_blobs[0]->count())";
  	else
    		memcpy(out, out_blobs[0]->cpu_data(), out_size * sizeof(float));
 }	

  //string/char* req_name?
  void* request_handler(string req_name, char * data){
	 map<string, Net<float>*>::iterator it = nets.find(req_name);
 	 if (it == nets.end()) {
   		 LOG(ERROR) << "Task " << req_name << " not found.";
   		 return (void*)1;
 	 } else
 		 LOG(INFO) << "Task " << req_name << " forward pass.";
	 
	 // reshape input dims if incoming data != current net config
	LOG(INFO) << "Elements received on socket " << sock_elts << endl;

	reshape(nets[req_name], sock_elts);

	int in_elts = nets[req_name]->input_blobs()[0]->count();
	int out_elts = nets[req_name]->output_blobs()[0]->count();
	float* in = (float*)malloc(in_elts * sizeof(float));
	float* out = (float*)malloc(out_elts * sizeof(float));
	
	// Main loop of the thread, following this order
	// 1. Receive input feature (has to be in the size of sock_elts)
	// 2. Do forward pass
	// 3. Send back the result
	// 4. Repeat 1-3

	// Warmup: used to move the network to the device for the first time
	// In all subsequent forward passes, the trained model resides on the
	// device (GPU)
	bool warmup = true;
 
	while (1) {
		LOG(INFO) << "Reading from socket.";
		//socket_send
		//type cast: float* in, void* data
		in = (char*) data;
	
    		if (warmup && gpu) {
			float loss;
      			vector<Blob<float>*> in_blobs = nets[req_name]->input_blobs();
      			in_blobs[0]->set_cpu_data(in);
      			vector<Blob<float>*> out_blobs;
      			out_blobs = nets[req_name]->ForwardPrefilled(&loss);
      			warmup = false;
   		 }	

   		 LOG(INFO) << "Executing forward pass.";
   		 SERVICE_fwd(in, in_elts, out, out_elts, nets[req_name]);

    		LOG(INFO) << "Writing to socket.";
		//out is sth that need to be sent back 
    		SOCKET_send(socknum, (char*)out, out_elts * sizeof(float), debug);
   		 //socket_receive
      }	

	// Exit the thread
	LOG(INFO) << "Socket closed by the client.";

	free(in);
	free(out);

	return (void*)0;
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







