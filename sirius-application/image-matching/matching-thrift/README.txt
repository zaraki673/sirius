In the file "detect.cpp" you have to change line 339 to make it point to the direct where the landmark db is(in red in the example)
Line 339:
 fs::path p = fs::system_complete("/home/hailong/sirius/sirius-application/image-matching/matching/landmarks/db");

2. go to the directory of matching-thrift and run make to compile the files
3. Right now the way it is setup there has to be a jpeg in the matching-thrift folder called "test.jpg"
4. when you execute ./run_imserver.sh, give it 30 secs to run before you initiate a request otherwise a thrift will throw a "connection refused"
5. I don't think you need to build the database but I did anyways
6.for some reason, the response that comes back is the actual file name complete with path and not the answer. so like if test.jpg is the Tower of Pisa, the response is /home/moeizr/~~/~~/tower.of.pisa.jpg
