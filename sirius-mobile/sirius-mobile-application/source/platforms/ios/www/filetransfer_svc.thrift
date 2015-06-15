# QueryType is the struct that clients send
# to inform the command center of what services they need.
struct QueryType
{
    1:bool ASR,
    2:bool QA,
    3:bool IMM
}

# File contains the string info and the string's format
struct File
{
    1:string file = "",
    2:bool b64format = false
}


# QueryData is the information that clients send
# when they wish to communicate with the command center.
struct QueryData
{
    1:string audioFile,
    2:string textFile,
    3:string imgFile
}

service FileTransferSvc {
    string ping(),
    void send_file(1: QueryData data, 2: QueryType qType, 3: string uuid),
    string get_response(1: string uuid),
}