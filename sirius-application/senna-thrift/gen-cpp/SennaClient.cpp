nclude <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <glog/logging.h>

#include "boost/program_options.hpp"
/*
#include "SENNA_utils.h"
#include "SENNA_Hash.h"
#include "SENNA_Tokenizer.h"
#include "SENNA_POS.h"
#include "SENNA_CHK.h"
#include "SENNA_NER.h"
*/

#include "socket.h"
#include "tonic.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "../gen-cpp/SennaService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace po = boost::program_options;

po::variables_map parse_opts(int ac, char **av) {
  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "Produce help message")
  ("common,c", po::value<string>()->default_value("../../common/"),"Directory with configs and weights")
  ("task,t", po::value<string>()->default_value("pos"),"Image task: pos (Part of speech tagging), chk (Chunking), ner (Name "
      "Entity Recognition)")
  ("network,n", po::value<string>()->default_value("pos.prototxt"),"Network config file (.prototxt)")
  ("weights,w", po::value<string>()->default_value("pos.caffemodel"),"Pretrained weights (.caffemodel)")
  ("input,i", po::value<string>()->default_value("input/small-input.txt"),"File with text to analyze (.txt)")
  ("djinn,d", po::value<bool>()->default_value(false),"Use DjiNN service?")
  ("hostname,o", po::value<string>()->default_value("localhost"),"Server IP addr")
  ("portno,p", po::value<int>()->default_value(8080), "Server port")
  ("gpu,g", po::value<bool>()->default_value(false), "Use GPU?")
  ("debug,v", po::value<bool>()->default_value(false),"Turn on all debug");

  po::variables_map vm;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << desc << "\n";
    exit(1);
  }
  return vm;
}


//SennaClient.cpp
int main(int argc, char *argv[]) {
  po::variables_map vm = parse_opts(argc, argv);
  
  boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  SennaServiceClient client(protocol);

  TonicInput tInput(); 
  //debug = vm["debug"].as<bool>();
  tInput.task = vm["task"].as<string>();
  tInput.network =
      vm["common"].as<string>() + "configs/" + vm["network"].as<string>();
  tInput.weights =
      vm["common"].as<string>() + "weights/" + vm["weights"].as<string>();
  tInput.input = vm["input"].as<string>();

  // DjiNN service or local?
  tInput.djinn = vm["djinn"].as<bool>();
  tInput.gpu = vm["gpu"].as<bool>();

  try {
    transport->open();

    client.senna_all(tInput);
    
    try {
      client.calculate(1, work);
      cout << "Whoa? We can divide by zero!" << endl;
    } catch (InvalidOperation& io) {
      cout << "InvalidOperation: " << io.why << endl;
      // or using generated operator<<: cout << io << endl;
    }

    transport->close();
  } catch (TException& tx) {
    
  }
}
