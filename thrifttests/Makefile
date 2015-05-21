CXX = g++
CXX_FLAGS = -Wall #-Wno-unused -Wno-sign-compare

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

TARGET_CLIENT = qaclient 

OBJECTS_CLIENT = QAService.o QAClient.o 

all: $(TARGET_CLIENT)

$(TARGET_CLIENT): $(OBJECTS_CLIENT)
	$(CXX) $(OBJECTS_CLIENT) -o $(TARGET_CLIENT) $(LINK_FLAGS)

QAService.o: gen-cpp/QAService.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

QAClient.o: QAClient.cpp
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

clean:
	rm -rf $(TARGET_CLIENT) *.o

.PHONY:	all
