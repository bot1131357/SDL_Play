#include "stdafx.h"
#include "SDLEntity.h"


SDLEntity::SDLEntity(string id):_id(id)
{
}


SDLEntity::~SDLEntity()
{
}

void SDLEntity::draw(SDL_Surface* gSurface)
{
	//cout << _id << ": Drawing something...\n";
}
