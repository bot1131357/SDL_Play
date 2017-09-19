# Path to your include files
INC=-I /usr/include/SDL2
# Path to your include files
LIB=
# lib files to be included
LIBRARIES= -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
# Other C flags
CFLAGS=-std=c++11 -pthread -fpermissive


all: test #ptest

test: 01_GameEngine_v2.cpp 
	g++ $^ -o $@ $(INC) $(LIB) $(CFLAGS) $(LIBRARIES)  

clean:
	rm -f test

# reset && g++ 01_GameEngine_v2.cpp -I /usr/include/SDL2 -std=c++11 -fpermissive -o app -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -pthread && ./app 