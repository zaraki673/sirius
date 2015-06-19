#Kaldi#
Kaldi is a speech recognition toolkit written in C++ that is freely available under the Apache license. 

Kaldi has been modified using Apache Thrift. There are two layers to Kaldi included in Sirius. 

First layer is a modified version of Kaldi which contains the source code that is compiled with the `compile-sirius-server.sh` 
in the sirius-applicatiion folder.

Second Layer is the Thrift Layer with allows RPC and IDL components combined such that Kaldi running on a server, the client 
can send requests from anywhere. 

###Basic Setup###
Command Center needs to be up and running. Once that is up an running follow series of commands from the kaldi-thrift folder:

```
$ make thrift
$ make 
```

Open a second terminal to this same location.

In the first one, start up the serve with this command:

`$ ./server`

In the second one, run the client with a .wav file:

`$ ./client ../../inputs/questions/what.is.the.speed.of.light.wav`
