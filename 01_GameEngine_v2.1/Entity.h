#pragma once

#include <string>
#include "SDL.h"
#include "GameConfig.h"
using namespace std;
class Entity
{
protected:
	string _id;

	float _x, _y, _x_, _y_, _vx, _vy, _a_x, _a_y;
	SDL_Rect _srcRect;
	SDL_Rect _dstRect;
public:
	Entity(string id, SDL_Rect& srcRect) :_id(id),_srcRect(srcRect){}
	virtual ~Entity() {}
	virtual void update() {};
	virtual void animate() {};

	string get_id() { return _id; };

	const float get_v_x() { return _vx; }
	const float get_v_y() { return _vy; }
	// current location
	const float get_x() { return _x; }
	const float get_y() { return _y; }
	// previous location
	const float get_x_() { return _x_; }
	const float& get_y_() { return _y_; }
	// get Rects
	const SDL_Rect& get_srcRect() { return _srcRect; }
	const SDL_Rect& get_dstRect() { return _dstRect; }
};

enum class FrameOrientation {Down, Right};
class AnimateFrames: public Entity
{
	GameConfig& gc;
	int _nframes;
	int _frame_idx;
	FrameOrientation _orientation;
public:
	AnimateFrames(string id, int nframes, SDL_Rect& srcRect, FrameOrientation orientation) : Entity(id, srcRect), _nframes(nframes), _orientation(orientation), _frame_idx(0), gc(GameConfig::getInstance())
	{
		_dstRect = { 0,0, gc.window_width, gc.window_height };
	}

	void animate()
	{
		const int frame_drag = gc.framerate*.5;
		static int drag_idx = 5;
		if (--drag_idx == 0)
		{
			drag_idx = frame_drag;
			if ((++_frame_idx) < _nframes)
			{
				switch (_orientation)
				{
				case FrameOrientation::Down: _srcRect.y += _srcRect.h; break;
				case FrameOrientation::Right: _srcRect.h += _srcRect.w; break;
				}
			}
		}
	}

};