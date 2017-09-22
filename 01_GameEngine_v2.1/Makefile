# Path to your include files
INC=-I /usr/include/SDL2 -I ~/Dev/Common/include
# Path to your include files
LIB=
# lib files to be included
LIBRARIES= -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
# Other C flags
CFLAGS=-std=c++11 -pthread -fpermissive

SRC=01_GameEngine_v2.1.cpp

all: test #ptest

test: $(SRC) 
	g++ $^ -o $@ $(INC) $(LIB) $(CFLAGS) $(LIBRARIES)  

testjs: $(SRC) 
	# source ~/Dev/emsdk-portable/emsdk_env.sh
	em++ $^ -o $@ $(INC) $(LIB) $(CFLAGS) $(LIBRARIES) -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 --preload-file ../sprites -o write_owl.html
clean:
	rm -f test

# reset && g++ 01_GameEngine_v2.cpp -I /usr/include/SDL2 -std=c++11 -fpermissive -o app -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -pthread && ./app 