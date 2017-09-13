#include "stdafx.h"
#include "SDLDisplayManager.h"




SDLDisplayManager::SDLDisplayManager(int width, int height, int framerate, SDL_Window* gWindow):
	_width(width), _height(height), _framerate(framerate), _gWindow(gWindow), _keepAlive(true)
{
	{
		//Get window surface
		_gScreenSurface = SDL_GetWindowSurface(_gWindow);
	}


	_thread_draw = thread([=]() {

		chrono::high_resolution_clock::time_point t1;
		chrono::high_resolution_clock::time_point t2;
		t1 = chrono::high_resolution_clock::now();
		while (_keepAlive)
		{
			//t2 = chrono::high_resolution_clock::now();
			//if ((t2 - t1) > chrono::milliseconds(1000 / framerate))
			//{
			//	t1 = t2;

			//	// Calls the registered draw functions
			//	{
			//		lock_guard<mutex> lg(mtx_drawfunctions);
			//		for (auto df : _draws)
			//		{
			//			df(_gScreenSurface);
			//		}
			//	}
			//	//Update the surface
			//	SDL_UpdateWindowSurface(_gWindow);
			//}

			
			//// Calls the registered draw functions
			//{
			//	lock_guard<mutex> lg(mtx_drawfunctions);
			//	for (auto df : _draws)
			//	{
			//		df(_gScreenSurface);
			//	}
			//}

			////Update the surface
			//SDL_UpdateWindowSurface(_gWindow);

			//t2 = chrono::high_resolution_clock::now();
			//SDL_Delay((chrono::milliseconds(1000 / framerate) - (t2 - t1)).count());
			//t1 = t2;

			
			// Calls the registered draw functions
			{
				lock_guard<mutex> lg(mtx_drawfunctions);
				for (auto df : _draws)
				{
					df(_gScreenSurface);
				}
			}

			//Update the surface
			SDL_UpdateWindowSurface(_gWindow);

			t2 = chrono::high_resolution_clock::now();
			this_thread::sleep_for(chrono::milliseconds(1000 / framerate) - (t2 - t1));
			t1 = t2;
		}
	});
}

SDLDisplayManager::~SDLDisplayManager()
{
	_keepAlive = false;
	if (_thread_draw.joinable()) _thread_draw.join();
}

void SDLDisplayManager::registerEntity(function<void(SDL_Surface*)> drawfunction)
{
	lock_guard<mutex> lg(mtx_drawfunctions);
	_draws.push_back(drawfunction);
}
