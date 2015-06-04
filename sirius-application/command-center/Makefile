CXX = g++
CXX_FLAGS =

LINK_FLAGS = -lopencv_core \
                         -lopencv_highgui \
                         -lopencv_imgproc \
                         -lopencv_nonfree \
                         -lopencv_flann \
                         -lopencv_objdetect \
                         -lopencv_features2d \
                         -lopencv_gpu \
                         -lrt \
                         -lboost_program_options \
                         -lboost_filesystem \
                         -lboost_system \
                         -lprotobuf \
                         -ltesseract \
			 -lthrift

TARGET_CLIENT = ccclient
TARGET_SERVER = ccserver

OBJECTS_CLIENT = gen-cpp/CommandCenter.o CmdCenterClient.o
OBJECTS_SERVER = ../question-answer/openephyra-thrift/QAService.o gen-cpp/CommandCenter.o CmdCenterDaemon.o 

all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_CLIENT): $(OBJECTS_CLIENT)
	$(CXX) $(OBJECTS_CLIENT) -o $(TARGET_CLIENT) $(LINK_FLAGS)

$(TARGET_SERVER): $(OBJECTS_SERVER)
	$(CXX) $(OBJECTS_SERVER) -o $(TARGET_SERVER) $(LINK_FLAGS)

../question-answer/openephyra-thrift/QAService.o: ../question-answer/openephyra-thrift/gen-cpp/QAService.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

gen-cpp/CommandCenter.o : gen-cpp/CommandCenter.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

CmdCenterClient.o : CmdCenterClient.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

CmdCenterDaemon.o : CmdCenterDaemon.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

clean:
	rm -rf $(TARGET_SERVER) *.o ../question-answer/openephyra-thrift/QAService.o gen-cpp/*.o

.PHONY:	all
