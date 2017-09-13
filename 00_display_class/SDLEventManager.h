#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <functional>

#include <SDL.h>
using namespace std;

class SDLEventManager
{
	mutex mtx_keypresses;
	bool _keepAlive;
	int _pollrate;
	thread _thread_poll;
	function<void(SDL_Event)> _eventHandler;
	function<void(SDL_Keycode)> _keyHandler;

	//The window we'll be rendering to
	SDL_Window* _gWindow = NULL;
	
public:
	SDLEventManager(int width, int height, int pollrate);
	~SDLEventManager();

	SDL_Window* getWindow() { return _gWindow; }
	void registerEventHandler(function<void(SDL_Event)> eventHandler);
	void registerKeyHandler(function<void(SDL_Keycode)> keyHandler);
	void registerMouseHandler(function<void(SDL_Event)> inputHandler);


};

