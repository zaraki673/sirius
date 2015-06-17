COMMON=~/djinn-master/common
include $(COMMON)/Makefile.config

CXX = g++
CXX_FLAGS  = -O3 \
                         $(INCLUDE_DIRS) \
                         -w \
                         -fpermissive \
                         -I$(COMMON)/include \
                         -I$(CAFFE)/include \
                         -I$(CUDA)/include

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
                         $(CAFFE)/lib/libcaffe.so

SRC=src

TARGET_CLIENT = sennaClient
TARGET_SERVER = sennaServer

INCLUDES_DIR = /home/jesswu/thriftSenna/sirius/sirius-application/senna-thrift/gen-cpp

# File names
EXEC = sennaClient
SOURCES = $(wildcard $(SRC)/*.cpp) $(COMMON)/src/tonic.cpp SennaClient.cpp SennaService.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Main target
$(EXEC): $(OBJECTS) Makefile ;  $(CXX) $(OBJECTS) -o $(EXEC) $(LINK_FLAGS)

SennaService.o : SennaService.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

# To obtain object files
%.o: %.cpp Makefile ;  $(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

# To remove generated files
clean: ; rm -f $(EXEC) $(SRC)/*.o $(COMMON)/src/*.o

