#Command Center
##Responsibilities:
- Maintains a list of registered services.
- Forwards a client's request to the appropriate registered services.
- Returns requested information to the client.


##Starting the command center:
1) Generate the client/server stubs
```thrift --gen cpp --gen java commandcenter.thrift```
(see <https://thrift.apache.org/>)

2) Compile the command center
```make```

3) Start the command center
```./ccserver <port>```


##Sending a request to the server:
1) Generate the command center thrift source code in your language of choice
```thrift --gen <language> commandcenter.thrift```

2) Step (1) will create a new directory, gen-<language>/ 
in this directory. Look at the generated code and include them
in your service. These files provide an interface for communicating with
the command center.

3) Create a CommandCenterClient object, and call its handleRequest() method
with your query type (eg. ASR-QA-IMM, ASR-QA), and your data. 
Check commandcenter.thrift or the generated source code if you're not sure how to make this call.


##Registering a service with the command center:
1) Generate the command center thrift source code in your language of choice
```thrift --gen <language> commandcenter.thrift```

2) Step (1) will create a new directory, gen-<language>/ 
in this directory. Look at the generated code and include them
in your service. These files provide an interface for communicating with
the command center.

3) Create a CommandCenterClient object, and call its registerService() method
with the appropriate arguments. Check commandcenter.thrift or the generated
source code if you're not sure how to make this call.

4) Link Libraries
(C++ services) When you compile your service, be sure to link in the
command center thrift binaries.

(Java services) When you compile/run your serve, make sure that the command
center java libraries are on your class path.
