# Makefile for generating the kaldi-thrift service
CXX = g++
TARGET_CLIENT = client
TARGET_SERVER = server
LINK_FLAGS = -lthrift \
	     -lsox

CMDOBJECT_SERVER = /home/momo/Research/sirius/sirius-application/command-center/gen-cpp/CommandCenter.o \
									/home/momo/Research/sirius/sirius-application/command-center/gen-cpp/commandcenter_types.o 
									
OBJECTS_CLIENT = KaldiService.o Kaldi_Client.o
OBJECTS_SERVER = KaldiService.o Kaldi_Server.o $(CMDOBJECT_SERVER) 
all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_CLIENT): $(OBJECTS_CLIENT)
	$(CXX) $(OBJECTS_CLIENT) -o $(TARGET_CLIENT) $(LINK_FLAGS)

$(TARGET_SERVER): $(OBJECTS_SERVER)
	$(CXX) $(OBJECTS_SERVER) -o $(TARGET_SERVER) $(LINK_FLAGS)

%.o: %.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

clean:
	rm -rf $(TARGET_CLIENT) $(TARGET_SERVER) *.o

.PHONY:	all
