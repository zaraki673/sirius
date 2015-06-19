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
#include "time"

//thread.h
#include <vector>
#include <map>
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

extern map<string, Net<float>*> nets;
extern bool debug;
extern bool gpu;


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
 
    if (app.task == "pos"){
	if (app.djinn){
		SENNA_POS_forward_basic(pos, tokens->word_idx, tokens->caps_idx,
                                   tokens->suff_idx, app);
		(char *)(pos->output_state) = request_handler(app.pl.req_name, (char *)app.			pl.data);
		 
		 pos->labels = SENNA_realloc(pos->labels, sizeof(int), app.pl.num);
  		SENNA_nn_viterbi(pos->labels, pos->viterbi_score_init,
                   pos->viterbi_score_trans, pos->output_state,
                   pos->output_state_size, app.pl.num);

	}else{
		reshape(app.net, app.pl.num * app.pl.size);
		SENNA_POS_forward_basic(pos, tokens->word_idx, tokens->caps_idx,
                                   tokens->suff_idx, app);
		pos_labels = SENNA_POS_forward_noDjiNN(pos, tokens->word_idx, tokens->caps_idx,
                                   tokens->suff_idx, app);
	}
    
    }	
    else if (app.tast == "chk"){
	 TonicSuiteApp pos_app = app;
	 pos_app.task = "pos";
   	 pos_app.network = vm["common"].as<string>() + "configs/" + "pos.prototxt";
   	 pos_app.weights = vm["common"].as<string>() + "weights/" + "pos.caffemodel";
    
    	if (!pos_app.djinn) {
     	 pos_app.net = new Net<float>(pos_app.network);
     	 pos_app.net->CopyTrainedLayersFrom(pos_app.weights);
   	 }
   	 strcpy(pos_app.pl.req_name, pos_app.task.c_str());
   	 pos_app.pl.size =
       		 pos->window_size *
       		 (pos->ll_word_size + pos->ll_caps_size + pos->ll_suff_size);

   	 if (pos_app.djinn){
		SENNA_POS_forward_basic(pos, tokens->word_idx, tokens->caps_			idx,tokens->suff_idx, pos_app);
                (char *)(pos_app->output_state) = request_handler(pos_app.pl.req			_name, (char *)pos_app.pl.data);

                 pos->labels = SENNA_realloc(pos->labels, sizeof(int), pos_app.p			l.num);
                SENNA_nn_viterbi(pos->labels, pos->viterbi_score_init,
                   pos->viterbi_score_trans, pos->output_state,
                   pos->output_state_size, pos_app.pl.num);
	}else{
		reshape(pos_app.net, pos_app.pl.num * pos_app.pl.size);
                SENNA_POS_forward_basic(pos, tokens->word_idx, tokens->caps_idx,
                                   tokens->suff_idx, pos_app);
                pos_labels = SENNA_POS_forward_noDjiNN(pos, tokens->word_idx, tokens->caps_idx,
                                   tokens->suff_idx, pos_app);
	}

	if (app.djinn){
		SENNA_CHK_forward_basic(chk, tokens->word_idx, tokens->caps_idx,
                                   pos_labels, app);
		(char*)(chk->output_state) = request_handler(app.pl.req_name ,(char *)app.pl.data);
		 chk->labels = SENNA_realloc(chk->labels, sizeof(int), app.pl.num);
 		 SENNA_nn_viterbi(chk->labels, chk->viterbi_score_init,
                   chk->viterbi_score_trans, chk->output_state,
                   chk->output_state_size, app.pl.num);
	}else{
		free(pos_app.net);
		reshape(app.net, app.pl.num * app.pl.size);
		SENNA_CHK_forward_basic(chk, tokens->word_idx, tokens->caps_idx,
                                   pos_labels, app);
		chk_labels = SENNA_CHK_forward_noDjiNN(chk, tokens->word_idx, tokens->caps_idx, pos_labels, app);
	}
 
    }else if (app.task == "ner"){
	if (app.djinn){
		SENNA_NER_forward_basic(ner, tokens->word_idx, tokens->caps_idx,
                                   tokens->gazl_idx, tokens->gazm_idx,
                                   tokens->gazo_idx, tokens->gazp_idx, app);
		(char*)(ner->output_state) = request_handler(app.pl.req_name, app.pl.data);		
   	   ner->labels = SENNA_realloc(ner->labels, sizeof(int), app.pl.num);
 	     SENNA_nn_viterbi(ner->labels, ner->viterbi_score_init,
                   ner->viterbi_score_trans, ner->output_state,
                   ner->output_state_size, app.pl.num);
   	}else{
		reshape(app.net, app.pl.num * app.pl.size);
		SENNA_NER_forward_basic(ner, tokens->word_idx, tokens->caps_idx,
                                   tokens->gazl_idx, tokens->gazm_idx,
                                   tokens->gazo_idx, tokens->gazp_idx, app);
		ner_labels = SENNA_NER_forward_noDjiNN(ner, tokens->word_idx, tokens->caps_idx,
                                   tokens->gazl_idx, tokens->gazm_idx,
                                   tokens->gazo_idx, tokens->gazp_idx, app);
		
	}
     }



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


    // clean up
    SENNA_Tokenizer_free(tokenizer);
 
    SENNA_POS_free(pos);
    SENNA_CHK_free(chk);
    SENNA_NER_free(ner);

    SENNA_Hash_free(word_hash);
    SENNA_Hash_free(caps_hash);
    SENNA_Hash_free(suff_hash);
    SENNA_Hash_free(gazt_hash);

    SENNA_Hash_free(gazl_hash);
    SENNA_Hash_free(gazm_hash);
    SENNA_Hash_free(gazo_hash);
    SENNA_Hash_free(gazp_hash);

    SENNA_Hash_free(pos_hash);

   if (!app.djinn) free(app.net);
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
 	
	//break up the while-loop
	//while (1) {
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
    		//SOCKET_send(socknum, (char*)out, out_elts * sizeof(float), debug);
   		 //socket_receive
      //}	

	// Exit the thread
	LOG(INFO) << "Socket closed by the client.";

	//free(in);
	//free(out);

	return (char*)out;
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







