function pingServer(){
    console.log("Creating client");
    var addr = '//localhost:8081';
    console.log(addr);
    var transport = new Thrift.Transport(addr);
    var protocol  = new Thrift.TBinaryProtocol(transport);
    var client    = new CommandCenterClient(protocol);
    client.ping();
    console.log("Client Created");
}
document.getElementById("pingServer").addEventListener("click",pingServer);
