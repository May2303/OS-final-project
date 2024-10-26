# Compiler
CC = g++

# Compiler flags
CFLAGS = -std=c++17 -Werror -Wsign-conversion -pthread

# Source files
graphSrc = $(wildcard Graph/*.cpp)
MSTSrc = $(wildcard MinSpaningTree/*.cpp)
STRUCTSrc = $(wildcard Struct/*.cpp) Struct/data_structures.hpp
APITSrc = $(wildcard Server_API/*.cpp)


lf-serverSrc = LF-Server.cpp LeaderFollower/LeaderFollower.cpp
PIPELINE = Pipeline-server.cpp PipelineActiveObject/pipelineActiveObject.cpp


# Object files
LF-OBJ = $(graphSrc:.cpp=.o) $(lf-serverSrc:.cpp=.o) $(MSTSrc:.cpp=.o) $(STRUCTSrc:.cpp=.o) $(APITSrc:.cpp=.o)
PIPELINE-OBJ = $(graphSrc:.cpp=.o) $(PIPELINE:.cpp=.o) $(MSTSrc:.cpp=.o) $(STRUCTSrc:.cpp=.o) $(APITSrc:.cpp=.o)


.PHONY: all  pipeline-server valgrind clean
all: lf-server pipeline-server 


lf-server: $(LF-OBJ)
	$(CC) $(CFLAGS) $(LF-OBJ) -o lf-server

pipeline-server: $(PIPELINE-OBJ)
	$(CC) $(CFLAGS) $(PIPELINE-OBJ) -o pipeline-server


# Compile source files without coverage flags
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@


# Clean build files
clean:
	rm -f -r *.o Graph/*.o MinSpaningTree/*.o Struct/*.o lf-server PIPELINE-server  LeaderFollower/*.o Server_API/*.o PipelineActiveObject/*.o pipeline-server

	
