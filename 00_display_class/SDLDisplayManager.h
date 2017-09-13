#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <functional>

#include <SDL.h>
#include <SDL_image.h>


#include "SDLEntity.h"

using namespace std;

//enum BackgroundMode { Tiled, Single };
class SDLDisplayManager
{
	mutex mtx_drawfunctions;
	bool _keepAlive;
	int _framerate;
	int _width; 
	int _height;
	thread _thread_draw;
	vector<function<void(SDL_Surface*)>> _draws;


	SDL_Window* _gWindow = NULL;
	//The window renderer
	//SDL_Renderer* _gRenderer = NULL;
	SDL_Surface* _gScreenSurface = NULL;
public:
	SDLDisplayManager(int width, int height, int framerate, SDL_Window* gWindow);
	~SDLDisplayManager();
	
	const SDL_Surface* getScreenSurface() { return _gScreenSurface; }
	void registerEntity(function<void(SDL_Surface*)> drawfunction);


};

