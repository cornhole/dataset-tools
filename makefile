# compiler command and flags
CC=g++ 
CFLAGS=-g -std=c++11
OFLAGS=-g -std=c++11 -c 
LDLIBS=`pkg-config --libs opencv`
OPENCVPATH=/usr/lib/x86_64-linux-gnu/
# variables
CAPTS=captureTrainingSample
OBJS=  jsoncpp.o

default: json captrain parse

json: ./jsoncpp/dist/jsoncpp.cpp  ./jsoncpp/dist/json/json.h
	$(CC) $(OFLAGS) -Ijsoncpp/include/include/json ./jsoncpp/dist/jsoncpp.cpp 

captrain: $(OBJS) $(CAPTS).cpp
	$(CC) $(CFLAGS) $(OBJS) $(CAPTS).cpp -lboost_system $(LDLIBS) -o captureNewImage

#parse json file to make sure it is correct with the images
parse: $(OBJS) parseJson.cpp
	$(CC) $(CFLAGS) $(OBJS) parseJson.cpp  $(LDLIBS) -o parseJson

clean:
	$(RM) captureNewImage parseJson *.o
