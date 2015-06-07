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

# As long as the object files are present in the specified locations,
# linking should work.
OBJECTS_CLIENT = gen-cpp/CommandCenter.o gen-cpp/commandcenter_types.o CmdCenterClient.o
OBJECTS_SERVER = ../openephyra-thrift/QAService.o \
		 ../speech-recognition/kaldi/scripts/kaldi-thrift/gen-cpp/KaldiService.o \
		../image-matching/matching-thrift/ImageMatchingService.o \
		gen-cpp/CommandCenter.o gen-cpp/commandcenter_types.o CmdCenterDaemon.o 

all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_CLIENT): $(OBJECTS_CLIENT)
	$(CXX) $(OBJECTS_CLIENT) -o $(TARGET_CLIENT) $(LINK_FLAGS)

$(TARGET_SERVER): $(OBJECTS_SERVER)
	$(CXX) $(OBJECTS_SERVER) -o $(TARGET_SERVER) $(LINK_FLAGS)

# FIXME: services should be responsible for making themselves
#../question-answer/openephyra-thrift/QAService.o: ../question-answer/openephyra-thrift/gen-cpp/QAService.cpp
#	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

gen-cpp/CommandCenter.o : gen-cpp/CommandCenter.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

gen-cpp/commandcenter_types.o : gen-cpp/commandcenter_types.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

CmdCenterClient.o : CmdCenterClient.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

CmdCenterDaemon.o : CmdCenterDaemon.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

clean:
	rm -rf $(TARGET_SERVER) ccclient ccserver *.o gen-cpp/*.o

.PHONY:	all
