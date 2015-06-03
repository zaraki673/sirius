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

  /**
   * A method definition looks like C code. It has a return type, arguments,
   * and optionally a list of exceptions that it may throw. Note that argument
   * lists and exception lists are specified using the exact same syntax as
   * field lists in struct or exception definitions.
   */
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
   * This method has a oneway modifier. That means the client only makes
   * a request and does not listen for any response at all. Oneway methods
   * must be void.
   * 
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

