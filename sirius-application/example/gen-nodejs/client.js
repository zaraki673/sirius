var thrift = require('thrift');
var ThriftTransports = require('thrift/lib/thrift/transport.js');
var ThriftProtocols = require('thrift/lib/thrift/protocol.js');
var ThriftWebServer = require('thrift/lib/thrift/web_server.js');
var Calculator = require('./hello_svc.js');
var ttypes = require('./helloworld_types.js');

var transport = ThriftTransports.TBufferedTransport();
// transport = ThriftWebServer.TWebSocketTransport();
var protocol = ThriftProtocols.TJSONProtocol();

var connection = thrift.createConnection("clarity04.eecs.umich.edu", 4444, {
  transport : transport,
  protocol : protocol
});

connection.on('error', function(err) {
  assert(false, err);
});

// Create a Calculator client with the connection
var client = thrift.createClient(Calculator, connection);


client.ping(function(err, response) {
  console.log('ping()');
});
