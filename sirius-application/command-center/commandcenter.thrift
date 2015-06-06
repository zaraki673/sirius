# This file outlines the command center's API.
# Thrift will generate stubs that handle serialization/deserialization
# during remote procedure calls.
# Services register with the command center using the registerService() function.

namespace cpp cmdcenterstubs
namespace java cmdcenterstubs

# MachineData is the information that services send
# when they wish to register with the command center.
struct MachineData
{
	1:string name,
	2:i32 port
}

# QueryData is the information that clients send
# when they wish to communicate with the command center.
struct QueryData
{
	1:string audioFile = "",
	2:string textFile = "",
	3:string imgFile = ""
}

service CommandCenter
{
	# service <--> command center API
	void registerService(1:string serviceType, 2:MachineData mDataObj),

	# command center <--> client API
	#string handleRequest(1:string queryType, 2:QueryData data),

	# deprecated
	string askTextQuestion(1:string question),

	# simple function to test connections
	void ping()
}
