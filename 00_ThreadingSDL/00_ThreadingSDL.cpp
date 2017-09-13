// 00_ThreadingSDL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <thread>
#include "SDL.h"
#include "SDL_image.h"

using namespace std;


const int framerate = 30;
const int pollrate = 10;
const int width = 400;
const int height = 300;

int main(int argc, char* argv[])
{
	SDL_Window* _gWindow;

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

	SDL_Surface* _gScreenSurface;

	//Get window surface
	_gScreenSurface = SDL_GetWindowSurface(_gWindow);

	SDL_Surface* _loadedSurface;
	SDL_Surface* tempSurface = IMG_Load("../sprites/sprites01.png");
	_loadedSurface = SDL_ConvertSurface(tempSurface, _gScreenSurface->format, NULL);
	SDL_FreeSurface(tempSurface);



	//The window renderer
	SDL_Renderer* _gRenderer = NULL;
	//Create renderer for window
	_gRenderer = SDL_CreateRenderer(_gWindow, -1, SDL_RENDERER_ACCELERATED);
	if (_gRenderer == NULL)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return -1;
	}
	//Initialize renderer color
	SDL_SetRenderDrawColor(_gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	SDL_Texture* newTexture = NULL;
	//Color key image
	SDL_SetColorKey(_loadedSurface, SDL_TRUE, SDL_MapRGB(_loadedSurface->format, 0, 0xFF, 0xFF));

	//Create texture from surface pixels
	newTexture = SDL_CreateTextureFromSurface(_gRenderer, _loadedSurface);
	if (newTexture == NULL)
	{
		return -1;
	}

	SDL_Rect srcRect = { 8 - 1	,8 - 1,76 ,211 }; srcRect.w -= srcRect.x; srcRect.h -= srcRect.y;
	SDL_Rect rect = { 0,0,srcRect.w,srcRect.h };
	//SDL_Rect rect = { 0,0,_loadedSurface->w,_loadedSurface->h };

	//Get rid of old loaded surface
	SDL_FreeSurface(_loadedSurface);


	bool quit = false;
	SDL_Event e;
	while (!quit)
	{
		if (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.sym)
				{
				case SDLK_UP:
					rect.y += 1; break;
				case SDLK_DOWN:
					rect.y -= 1; break;
				default:
					break;
				}
			}
		}
		//Apply the image
		//SDL_BlitSurface(_loadedSurface, NULL, _gScreenSurface, &rect);
		SDL_RenderCopy(_gRenderer, newTexture, NULL, &rect);

		SDL_RenderPresent(_gRenderer);
		//Update the surface
		//SDL_UpdateWindowSurface(_gWindow);
	}




	return 0;
}

