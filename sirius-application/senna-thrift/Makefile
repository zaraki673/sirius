COMMON=/home/jesswu/djinn-master/common
CAFFE=/home/jesswu/caffe-0.9-beta
include $(COMMON)/Makefile.config 

CXX = g++
CXX_FLAGS  = -O3 \
                         $(INCLUDE_DIRS) \
                         -w \
                         -fpermissive \
                         -I$(COMMON)/include \
                         -I$(CAFFE)/include \
                         -I$(CUDA)/include \
                         -I/usr/local/include

ifeq ($(CPU_ONLY), 1)
    CXX_FLAGS  += -DCPU_ONLY
endif

LINK_FLAGS = $(LIBRARY_DIRS) \
                         $(COMMON)/libtonic.a \
                         -lboost_program_options \
                         -lboost_filesystem \
                         -lboost_system \
                         -lglog \
                         -lrt \
                         -lpthread \
						 -lthrift \
                         $(CAFFE)/lib/libcaffe.so

SRC=src
GEN=gen-cpp
LIBS=-lthrift

TARGET_CLIENT = sennaClient
TARGET_SERVER = sennaServer

INCLUDE_DIRS = /home/jesswu/thriftSenna/sirius/sirius-application/senna-thrift/gen-cpp


# File names
EXEC = sennaClient
SOURCES = $(wildcard $(SRC)/*.cpp) $(wildcard $(GEN)/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o) 

SERVER_OBJ = $(OBJECTS) SennaServer.o
CLIENT_OBJ = $(SOURCES:.cpp=.o) sennaClient.o

# Main target
#$(EXEC): $(OBJECTS) Makefile ;  $(CXX) $(OBJECTS) -o $(EXEC)  $(LIBS) $(LINK_FLAGS)
all: $(TARGET_SERVER) 

$(TARGET_SERVER): $(SERVER_OBJ) SennaServer.cpp ;
	 $(CXX) $(SERVER_OBJ) -o $(EXEC)  $(LIBS) $(LINK_FLAGS)

$(TARGET_CLIENT): $(CLIENT_OBJ) ;
	 $(CXX) $(CLIENT_OBJ) -o $(EXEC)  $(LIBS) $(LINK_FLAGS)



# To obtain object files
%.o: %.cpp Makefile ;  $(CXX) -I$(INCLUDE_DIRS) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

# To remove generated files
clean: ; rm -f $(EXEC) *.o 

