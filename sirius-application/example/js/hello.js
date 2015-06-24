function thriftMessage(){
    console.log("Creating client");
    var addr = 'https://clarity04.eecs.umich.edu:8081/cmdcenter';
    console.log(addr);
    var transport = new Thrift.TXHRTransport(addr);
    var protocol  = new Thrift.TJSONProtocol(transport);
    var client    = new hello_svcClient(protocol);
    client.ping();
    console.log("Client Created");
}
document.getElementById("thriftMessage").addEventListener("click",thriftMessage);