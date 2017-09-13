// 00_display_class.cpp : Defines the entry point for the console application.
//
// This is my first attempt to build a game engine, starting with display and input.
//What I like about the design is that all entities have a draw function that can be registered to the display manager. 
//I also liked that each entities have their textures and draw functions stored in them.
//However, SDL design expects certain types of structure. For example:
//- initvideo and pumpevent  needs to be in the main thread, or it will freeze the window




#include "stdafx.h"
#include "SDLDisplayManager.h"
#include "SDLEventManager.h"
#include "SDLEntity.h"
#include <thread>

const int framerate = 30;
const int pollrate = 10;
const int width = 400;
const int height = 300;


class Background : public SDLEntity
{
	SDL_Surface* _loadedSurface;
	SDL_Rect rect;

public:
	Background(string id, const SDL_Surface* gScreenSurface, string imgpath) :
		SDLEntity(id)
	{
		rect = { 0,0,width,height };
		SDL_Surface* tempSurface = IMG_Load(imgpath.c_str());
		if (tempSurface == NULL)
		{
			throw runtime_error(string("Failed to load surface") + SDL_GetError() + "\n");
		}
		_loadedSurface = SDL_ConvertSurface(tempSurface, gScreenSurface->format, NULL);
		SDL_FreeSurface(tempSurface);
	}
	~Background()
	{
		if(NULL!= _loadedSurface)
		SDL_FreeSurface(_loadedSurface);
	}
	void draw(SDL_Surface* gScreenSurface)
	{
		//Apply the image
		SDL_BlitSurface(_loadedSurface, NULL, gScreenSurface, &rect);
	}
	void update()
	{
		rect.x = (rect.x < width) ? rect.x + 1 : -width;
	}

};
class Critter : public SDLEntity
{
	SDL_Surface* _loadedSurface;
	SDL_Rect rect;
	SDL_Rect srcRect;

	float v_x, v_y;
	float a_x, a_y;

public:
	Critter(string id, const SDL_Surface* gScreenSurface, string imgpath) :
		SDLEntity(id)
	{
		SDL_Surface* tempSurface = IMG_Load(imgpath.c_str());
		if (tempSurface == NULL)
		{
			throw runtime_error(string("Failed to load surface") + SDL_GetError() + "\n");
		}

		srcRect = { 8 - 1	,8 - 1,76 ,211 }; srcRect.w -= srcRect.x; srcRect.h -= srcRect.y;
		//rect = { 10	,100,tempSurface->w,tempSurface->h };
		rect = { 10	,100,srcRect.w,srcRect.h };
		_loadedSurface = SDL_ConvertSurface(tempSurface, gScreenSurface->format, NULL);
		SDL_FreeSurface(tempSurface);
	}
	~Critter()
	{
		if(NULL!= _loadedSurface)
		SDL_FreeSurface(_loadedSurface);
	}
	void draw(SDL_Surface* gScreenSurface)
	{
		//Apply the image
		SDL_BlitSurface(_loadedSurface, &srcRect, gScreenSurface, &rect);
	}
	void update()
	{
		// Update location
		rect.x += v_x;
		rect.y += v_y;
		rect.x = (rect.x < width) ? rect.x : width-1;
		rect.x = (rect.x > 0) ? rect.x : 0;
		rect.y = (rect.y < height) ? rect.y : height - 1;
		rect.y = (rect.y > 0) ? rect.y : 0;
		//cout << rect.x << ", " << rect.y << endl;

		// Deccelerate
		v_x = (v_x < 0.0001 && v_x > -0.0001) ? 0 : .9*v_x;
		v_y = (v_y < 0.0001 && v_y > -0.0001) ? 0 : .9*v_y;
		//cout << v_x << ", " << v_y << endl;
	}
	void accel(int a_x, int a_y)
	{
		cout << "accel\n";
		v_x += a_x;
		v_y += a_y;
	}
};

int main(int argc, char* argv[])
{
	SDLEventManager EveMan(width, height, pollrate);
	SDLDisplayManager DispMan(width,height,framerate, EveMan.getWindow());

	SDLEntity anObject("testobj");
	Background bg("Background", DispMan.getScreenSurface(), "../sprites/bg01.png");
	Critter critter("Critter", DispMan.getScreenSurface(), "../sprites/sprites01.png");
	// Register entities that require drawing
	DispMan.registerEntity(bind(&Background::draw, &bg, placeholders::_1));
	DispMan.registerEntity(bind(&Critter::draw, &critter, placeholders::_1));
	DispMan.registerEntity(bind(&SDLEntity::draw, &anObject, placeholders::_1));

	bool quit = false;
	EveMan.registerEventHandler([&](SDL_Event event) {
		if (event.type == SDL_QUIT)
			quit = true;
	});
	EveMan.registerKeyHandler([&](SDL_Keycode keycode) {
		cout << "Key: " << keycode << endl;
		switch (keycode)
		{
		case SDLK_UP:
			critter.accel(0, -10); break;
		case SDLK_DOWN:
			critter.accel(0, 10); break;
		case SDLK_LEFT:
			critter.accel(-10,0); break;
		case SDLK_RIGHT:
			critter.accel(10, 0); break;
		default: break;
		}
	});

	SDL_Event e;
	//for (int i = 0; i < 10000; ++i)
	while(!quit)
	{
		SDL_PumpEvents();
		//if (SDL_PollEvent(&e) != 0)
		//{
		//	if (e.type == SDL_KEYDOWN)
		//	{
		//		cout << "Key:" << e.key.keysym.sym << endl;
		//		switch (e.key.keysym.sym)
		//		{
		//			case SDLK_UP: 
		//				critter.accel(0, 1); break;
		//			case SDLK_DOWN: 
		//				critter.accel(0, -1); break;
		//			default: break;
		//		}
		//	}
		//}
		
		//cout << ".";
		bg.update();
		critter.update();
		//SDL_Delay(100);
		this_thread::sleep_for(chrono::milliseconds(10));
	}

	//this_thread::sleep_for(chrono::seconds(10));

	//int x = 0;
	//int y = 0;
	//for (int i = 0; i < 10; ++i)
	//{
	//	++x;
	//	++y;
	//	DispMan.backgroundMove(x, y);

	//	this_thread::

	//}





    return 0;
}

