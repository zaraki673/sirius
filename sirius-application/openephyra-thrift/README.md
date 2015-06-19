##Open Ephyra

Open Ephyra is an open-source framework for question answering
developed at Carnegie Mellon.

Two versions of Open Ephyra are included in Sirius. The original version
can be compiled by running `./compile-sirius-servers.sh` in sirius-application/.

The version in this directory uses Apache Thrift. It belongs to a collection of services that
connect with the new Sirius command center.

####Basic Setup
Generate client/server stubs:

```
thrift --gen cpp qaservice.thrift
thrift --gen java qaservice.thrift
```

Compile server:

`./compile-qa-server-thrift.sh`

Start server:

PORT = the port you are starting on

CMDCENTERPORT = the port the command center has started on

`./start-qa-server-thrift.sh (PORT) (CMDCENTER-PORT)`

####Troubleshooting
Error message:

  `org.apache.thrift.transport.TTransportException: java.net.ConnectException: Connection refused`

Solution:  The QA service automatically tries to register with the command center
  at the port specified in start-qa-server-thrift.sh.
  If the command center is not running, then the registration fails,
  and this message is reported.
  
  To solve this problem, first start the command center server
  located at ~/sirius/sirius-application/command-center
