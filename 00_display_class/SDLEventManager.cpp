#include "stdafx.h"
#include "SDLEventManager.h"
#include <iostream>
using namespace std;

SDLEventManager::SDLEventManager(int width, int height, int pollrate):
	_pollrate(pollrate), _keepAlive(true)
{

	_keyHandler = bind([](SDL_Keycode k) {}, placeholders::_1);
	_eventHandler = bind([](SDL_Event e) {cout << "Event\n"; }, placeholders::_1);

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw runtime_error("Failed to initialise SDL\n");
	}
	//Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}

	//Create window
	_gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	if (_gWindow == NULL)
	{
		throw runtime_error(string("Failed to create Window") + SDL_GetError() + "\n");
	}

	_thread_poll = thread([&]() {

		chrono::high_resolution_clock::time_point t1;
		chrono::high_resolution_clock::time_point t2;
		SDL_Keycode keycode=SDLK_UNKNOWN;
		
		t1 = chrono::high_resolution_clock::now();
		while (_keepAlive)
		{
			// Calls the registered draw functions
			{
				SDL_Event e;
				if (SDL_PollEvent(&e) != 0)
				{
					t2 = chrono::high_resolution_clock::now();
					switch (e.type) {
					case SDL_KEYDOWN:
						if (keycode != e.key.keysym.sym || t2 - t1 > chrono::milliseconds(1000 / _pollrate))
						{
							t1 = t2;
							cout << "EveMan\n";
							keycode = e.key.keysym.sym;
							_keyHandler(keycode);
						}
						break;
					case SDL_QUIT:						
					default:
						_eventHandler(e);
					}
				}
			}			
		}
	});

}


SDLEventManager::~SDLEventManager()
{
	_keepAlive = false;
	if (_thread_poll.joinable()) _thread_poll.join();
}

void SDLEventManager::registerEventHandler(function<void(SDL_Event)> eventHandler)
{
	_eventHandler = eventHandler;
}

void SDLEventManager::registerKeyHandler(function<void(SDL_Keycode)> keyHandler)
{
	_keyHandler = keyHandler;
}

void SDLEventManager::registerMouseHandler(function<void(SDL_Event)> inputHandler)
{
}


