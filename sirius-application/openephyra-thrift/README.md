##Question Answer Thrift implementation
######Benjamin Toll
######Part of the Sirius suite

1. Generate client/server stubs:
`thrift --gen cpp qaservice.thrift`
`thrift --gen java qaservice.thrift`

2. Compile server:
`./compile-qa-server-thrift.sh`

3. Start server:
`./start-qa-server-thrift.sh`

####Troubleshooting
###Errors when starting the server:
-Problem as of 06/07/15
  -Error message:
  `org.apache.thrift.transport.TTransportException: java.net.ConnectException: Connection refused`

  -Solution:
  The QA service automatically tries to register with the command center
  at the port specified in start-qa-server-thrift.sh.
  If the command center is not running, then the registration fails,
  and this message is reported.
  To solve this problem, first start the command center server
  located at ~/sirius/sirius-application/command-center
