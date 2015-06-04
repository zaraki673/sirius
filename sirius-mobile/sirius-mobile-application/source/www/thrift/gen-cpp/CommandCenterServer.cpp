#include "CommandCenter.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using boost::shared_ptr;

class CommandCenterHandler : virtual public CommandCenterIf {
 public:
  CommandCenterHandler() {
    // Your initialization goes here
  }

  bool ping() {
    // Your implementation goes here
    printf("ping\n");
    return true;
  }

  void sendRequest(std::string& _return, const Request_Type::type type, const std::map<std::string, std::string> & data) {
    // Your implementation goes here
    printf("sendRequest\n"); 
  }

  /**
   * @param type
   * @param ip
   * @param port
   */
  void registerService(const Service_Type::type type, const std::string& ip, const std::string& port) {
    // Your implementation goes here
    printf("registerService\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<CommandCenterHandler> handler(new CommandCenterHandler());
  shared_ptr<TProcessor> processor(new CommandCenterProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

