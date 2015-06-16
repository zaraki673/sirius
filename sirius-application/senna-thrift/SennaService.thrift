struct TonicInput{
    1:string task = "pos",
    2:string network = "pos.prototxt",
    3:string weights = "pos.caffemodel",
    4:string input = "input/small-input.txt",
    5:bool gpu = false,
    6:bool djinn = false,
    7:string hostname = "localhost",
    8:i32 portno = 8080,
    9:i32 socketfd = 0
}

service SennaService{
    string senna_all(1:TonicInput tInput);
}
