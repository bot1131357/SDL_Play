#pragma once
#include <iostream>
#include <string>
#include <SDL_image.h>


using namespace std;
class SDLEntity
{
	string _id;
public:
	SDLEntity(string id);
	virtual ~SDLEntity();
	virtual void draw(SDL_Surface* gSurface);
};

