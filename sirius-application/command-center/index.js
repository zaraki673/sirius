function pingServer(){
    console.log("Creating client");
    var addr = 'http://localhost:8081';
    console.log(addr);
    var transport = new Thrift.Transport(addr);
    var protocol  = new Thrift.Protocol(transport);
    var client    = new CommandCenterClient(protocol);
    client.ping();
    console.log("Client Created");
}
document.getElementById("pingServer").addEventListener("click",pingServer);