// 01_GameEngine_v2.cpp : Defines the entry point for the console application.
//
// 

#include "stdafx.h"
#include <iostream>
#include <iomanip> 
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <bitset>
#include <algorithm>    // std::remove_if
#include "SDL.h"
#include "SDL_image.h"
#include "01_GameEngine_v2.h"

using namespace std;

const int framerate = 30;
const int pollrate = 60;
const int width = 400;
const int height = 300;

enum Collision { 
	Up, 
	Down, 
	Left,
	Right, 
	Collided
};
template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

class SDLEventManager
{
	atomic<bool> _keepRunning;
	int _pollrate;
	thread _eventLoop;
	atomic<SDL_Window*> _gWindow=NULL;
	SDL_Surface* _gScreenSurface = NULL;


	function<void(SDL_Event)> _eventHandler;
	function<void(SDL_Keycode)> _keyDownHandler;
	function<void(SDL_Keycode)> _keyUpHandler;

public:
	SDLEventManager(int pollrate, int width, int height):
		_pollrate(pollrate),_keepRunning(true)
	{

		_keyDownHandler = bind([](SDL_Keycode k) {}, placeholders::_1);
		_keyUpHandler = bind([](SDL_Keycode k) {}, placeholders::_1);
		_eventHandler = bind([](SDL_Event e) {cout << "Event\n"; }, placeholders::_1);

		_eventLoop = thread([&]() {
			//SDL_Window* _gWindow;

			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
			{
				throw runtime_error("Failed to initialise SDL\n");
			}
			//Set texture filtering to linear
			if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
			{
				printf("Warning: Linear texture filtering not enabled!");
			}

			//Create window
			_gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
			if (_gWindow.load() == NULL)
			{
				throw runtime_error(string("Failed to create Window") + SDL_GetError() + "\n");
			}

			//SDL_MaximizeWindow(_gWindow);

			chrono::high_resolution_clock::time_point t1;
			chrono::high_resolution_clock::time_point t2;
			SDL_Keycode keycode = SDLK_UNKNOWN;

			t1 = chrono::high_resolution_clock::now();
			SDL_Event e;
			while (_keepRunning)
			{
				if (SDL_PollEvent(&e) != 0)
				{
					t2 = chrono::high_resolution_clock::now();
					switch (e.type) {
					case SDL_KEYDOWN:
						if (keycode != e.key.keysym.sym || t2 - t1 > chrono::milliseconds(1000 / _pollrate))
						{
							t1 = t2;
							//cout << "EveMan\n";
							keycode = e.key.keysym.sym;
							_keyDownHandler(keycode);
						}
						break;
					case SDL_KEYUP:
						t1 = t2;
						//cout << "EveMan\n";
						keycode = e.key.keysym.sym;
						_keyUpHandler(keycode);
						break;
					case SDL_QUIT:
					default:
						_eventHandler(e);
					}
				}
			}
		});
	}
	~SDLEventManager() {
		_keepRunning = false;
		if(_eventLoop.joinable())_eventLoop.join();
	}
	SDL_Window* getWindow() {
		while (NULL == _gWindow.load())
		{
			this_thread::sleep_for(chrono::milliseconds(10));
		}
		return _gWindow.load();
	}

	void SDLEventManager::registerEventHandler(function<void(SDL_Event)> eventHandler)
	{
		_eventHandler = eventHandler;
	}

	void SDLEventManager::registerKeyHandler(function<void(SDL_Keycode)> keyDownHandler, function<void(SDL_Keycode)> keyUpHandler)
	{
		_keyDownHandler = keyDownHandler;
		_keyUpHandler = keyUpHandler;
	}

	void SDLEventManager::registerMouseHandler(function<void(SDL_Event)> inputHandler)
	{
		// stub
	}

};

#include <condition_variable>
class SDLDisplayManager
{
	std::mutex mtx_drawing;
	std::condition_variable cv_drawing;
	bool keepDrawing = false;

	mutex mtx_drawfunctions;
	bool _keepAlive;
	int _framerate;
	int _width;
	int _height;
	thread _thread_draw;
	vector<function<void(void)>> _draws;


	SDL_Window* _gWindow = NULL;
	SDL_Renderer* _gRenderer = NULL;
	SDL_Surface* _gScreenSurface = NULL;
public:
	SDLDisplayManager(int width, int height, int framerate, SDL_Window* gWindow) :
		_width(width), _height(height), _framerate(framerate), _gWindow(gWindow), _keepAlive(true)
	{
		//Get window surface
		_gScreenSurface = SDL_GetWindowSurface(_gWindow);

		//Create renderer for window
		_gRenderer = SDL_CreateRenderer(_gWindow, -1, SDL_RENDERER_ACCELERATED);

		//Initialize renderer color
		SDL_SetRenderDrawColor(_gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	}

	~SDLDisplayManager(){
		_keepAlive = false;
		if (_thread_draw.joinable()) _thread_draw.join();
	}

	void startDrawing()
	{
		_keepAlive = true;
		_thread_draw = thread([=]() {

			chrono::high_resolution_clock::time_point t1;
			chrono::high_resolution_clock::time_point t2;
			t1 = chrono::high_resolution_clock::now();
			while (_keepAlive)
			{
				// Calls the registered draw functions
				{
					lock_guard<mutex> lg(mtx_drawfunctions);
					for (auto df : _draws)
					{
						df();
					}
				}

				// Update rendered screen
				SDL_RenderPresent(_gRenderer);

				t2 = chrono::high_resolution_clock::now();
				this_thread::sleep_for(chrono::milliseconds(1000 / framerate) - (t2 - t1));
				t1 = t2;
			}
		});
	}
	void stopDrawing()
	{
		_keepAlive = false;
		_thread_draw.join();
	}
	const SDL_Surface* getScreenSurface() { return _gScreenSurface; }
	SDL_Renderer* getScreenRenderer() { return _gRenderer; }
	void registerEntity(function<void(void)> drawfunction) {
		lock_guard<mutex> lg(mtx_drawfunctions);
		_draws.push_back(drawfunction);
	}


};
class SDLEntity
{
protected:
	string _id;
public:
	SDL_Rect dstRect;
	//SDL_Rect dstRect_; // previous location
	float x, y, x_, y_; // better use Point2D
	float _v_x, _v_y;
	float _a_x, _a_y;
	SDLEntity::SDLEntity(string id) :_id(id)
	{
	}
	virtual ~SDLEntity() {}
	virtual void draw(SDL_Surface* gSurface) {}
	friend void handleCollision(SDLEntity &ref, SDLEntity &obj);
	friend bitset<5> detectCollision(SDLEntity &ref, SDLEntity &obj);

	const float get_v_x() { return _v_x; }
	const float get_v_y() { return _v_y; }
	// current location
	const float get_x() { return x; }
	const float get_y() { return y; }
	// previous location
	const float get_x_() { return x_; }
	const float& get_y_() { return y_; }
};
class IntroScreen : public SDLEntity
{
	SDL_Renderer* _gRenderer = NULL;
	SDL_Texture* _texture = NULL;
	SDL_Rect srcRect;
	const int imheight = 480;
	const int imwidth = 640;
public:
	IntroScreen(string id, SDL_Renderer* gRenderer, string imgpath) :
		SDLEntity(id), _gRenderer(gRenderer)
	{
		srcRect = { 0,0,imwidth,imheight };
		dstRect = { 0,0,width,height };
		SDL_Surface* tempSurface = IMG_Load(imgpath.c_str());
		if (tempSurface == NULL)
		{
			throw runtime_error(string("Failed to load surface") + SDL_GetError() + "\n");
		}

		//Create texture from surface pixels
		_texture = SDL_CreateTextureFromSurface(_gRenderer, tempSurface);
		SDL_FreeSurface(tempSurface);
	}
	~IntroScreen()
	{
		if (NULL != _texture)
			SDL_DestroyTexture(_texture);
	}
	void draw()
	{
		SDL_RenderCopy(_gRenderer, _texture, &srcRect, &dstRect);
	}
	void update()
	{
		srcRect.y += imheight;
	}
};
class GameOverScreen : public SDLEntity
{
	SDL_Renderer* _gRenderer = NULL;
	SDL_Texture* _texture = NULL;
	SDL_Rect srcRect;
	const int imheight = 480;
	const int imwidth = 640;
public:
	GameOverScreen(string id, SDL_Renderer* gRenderer, string imgpath) :
		SDLEntity(id), _gRenderer(gRenderer)
	{
		srcRect = { 0,0,imwidth,imheight };
		dstRect = { 0,0,width,height };
		SDL_Surface* tempSurface = IMG_Load(imgpath.c_str());
		if (tempSurface == NULL)
		{
			throw runtime_error(string("Failed to load surface") + SDL_GetError() + "\n");
		}

		//Create texture from surface pixels
		_texture = SDL_CreateTextureFromSurface(_gRenderer, tempSurface);
		SDL_FreeSurface(tempSurface);
	}
	~GameOverScreen()
	{
		if (NULL != _texture)
			SDL_DestroyTexture(_texture);
	}
	void draw()
	{
		SDL_RenderCopy(_gRenderer, _texture, &srcRect, &dstRect);
	}
	void update()
	{

	}
};
class Background : public SDLEntity
{
	SDL_Renderer* _gRenderer = NULL;
	SDL_Texture* _texture = NULL;

public:
	Background(string id, SDL_Renderer* gRenderer, string imgpath) :
		SDLEntity(id), _gRenderer(gRenderer)
	{
		dstRect = { 0,0,width,height };
		SDL_Surface* tempSurface = IMG_Load(imgpath.c_str());
		if (tempSurface == NULL)
		{
			throw runtime_error(string("Failed to load surface") + SDL_GetError() + "\n");
		}

		//Create texture from surface pixels
		_texture = SDL_CreateTextureFromSurface(_gRenderer, tempSurface);
		SDL_FreeSurface(tempSurface);
	}
	~Background()
	{
		if (NULL != _texture)
			SDL_DestroyTexture(_texture);
	}
	void draw()
	{
		SDL_RenderCopy(_gRenderer, _texture, NULL, &dstRect);
	}
	void update()
	{
		//dstRect.x = (dstRect.x < width) ? dstRect.x + 1 : -width;
	}

};

#include <random>
std::random_device r;
std::default_random_engine e1(r());
std::uniform_int_distribution<int> uniform_dist_w(1, width);
std::uniform_int_distribution<int> uniform_dist_h(1, height);
std::uniform_int_distribution<int> uniform_dist_x(1- width / 4, width/4);
std::uniform_int_distribution<int> uniform_dist_y(height / 2, height/2);

class Ball : public SDLEntity
{
	SDL_Renderer* _gRenderer = NULL;
	SDL_Texture* _texture = NULL;
	string _imgpath;
	//SDL_Surface* _loadedSurface;
	SDL_Rect srcRect;

	float MAX_VEL;
	float MIN_VEL;
public:
	Ball(string id, SDL_Renderer* gRenderer, string imgpath) :
		SDLEntity(id), _gRenderer(gRenderer), _imgpath(imgpath)
	{
		SDL_Surface* tempSurface = IMG_Load(_imgpath.c_str());
		if (tempSurface == NULL)
		{
			throw runtime_error(string("Failed to load surface") + SDL_GetError() + "\n");
		}
		x = uniform_dist_w(e1);
		y = uniform_dist_h(e1);
		//x = y = 100;
		srcRect = { 9 - 1	,134 - 1,12,12 };
		dstRect = { (int)x	,(int)y,12,12 };
		x_ = x;
		y_ = y;

		SDL_SetColorKey(tempSurface, SDL_TRUE, SDL_MapRGB(tempSurface->format, 0xFF, 0xFF, 0xFF));
		//Create texture from surface pixels
		_texture = SDL_CreateTextureFromSurface(_gRenderer, tempSurface);
		SDL_FreeSurface(tempSurface);
		_v_x = _v_y = -100.0 / pollrate;
		_a_x = _a_y = 0.0;

		MAX_VEL = 200.0 / pollrate;
		MIN_VEL = 50.0 / pollrate;
	}
	// Move
	Ball(Ball&& other) :SDLEntity(other._id)
	{
		// release the current object’s resources
		_gRenderer = other._gRenderer;
		_texture = other._texture;
		// Make sure the destructor during move don't destroy the texture
		other._gRenderer = NULL;
		other._texture = NULL;

		x = other.x;
		y = other.y;
		_v_x = other._v_x;
		_v_y = other._v_y;
		_a_x = other._a_x;
		_a_y = other._a_y;

		dstRect = other.dstRect;
		srcRect = other.srcRect;
		x_ = other.x;
		y_ = other.y;


		MAX_VEL = other.MAX_VEL;
		MIN_VEL = other.MIN_VEL;
	}
	Ball& operator=(Ball&& other)
	{
		// release the current object’s resources
		this->_gRenderer = other._gRenderer;
		this->_texture = other._texture;
		// Make sure the destructor during move don't destroy the texture
		other._gRenderer = NULL;
		other._texture = NULL;

		this->x = other.x;
		this->y = other.y;
		this->_v_x = other._v_x;
		this->_v_y = other._v_y;
		this->_a_x = other._a_x;
		this->_a_y = other._a_y;

		this->dstRect = other.dstRect;
		this->srcRect = other.srcRect;

		this->x_ = this->x;
		this->y_ = this->y;
		this->MAX_VEL = other.MAX_VEL;
		this->MIN_VEL = other.MIN_VEL;
		return *this;
	}
	~Ball()
	{
		if (NULL != _texture)
			SDL_DestroyTexture(_texture);
	}
	void draw()
	{
		SDL_RenderCopy(_gRenderer, _texture, &srcRect, &dstRect);
	}
	void update()
	{
		x_ = x;
		y_ = y;

		_v_x += _a_x;
		_v_y += _a_y;

		// Update location
		x += _v_x;
		y += _v_y;

		const bool BALL_BOUNCES_ON_WALL = false;
		const int LEFT_BOUND = 0;
		const int RIGHT_BOUND = width;
		if (BALL_BOUNCES_ON_WALL)
		{
			if (x < LEFT_BOUND) if (_v_x < 0)  _v_x = -_v_x;
			if (x + dstRect.w > RIGHT_BOUND) if (_v_x > 0) _v_x = -_v_x;
		}
		if (y < 0) if (_v_y <0)  _v_y = -_v_y;
		if (y + dstRect.h > height) if (_v_y > 0) _v_y = -_v_y;

		dstRect.x = x;
		dstRect.y = y; 

		// Check speed limit
		_v_y = (_v_y < -MAX_VEL || _v_y > MAX_VEL) ? sgn(_v_y)*MAX_VEL : _v_y;
		_v_x = (_v_x < -MAX_VEL || _v_x > MAX_VEL) ? sgn(_v_x)*MAX_VEL : _v_x;

		// Deccelerate
		_v_y = (_v_y < MIN_VEL && _v_y > -MIN_VEL) ? sgn(_v_y)*MIN_VEL : .999*_v_y;
		_v_x = (_v_x < MIN_VEL && _v_x > -MIN_VEL) ? sgn(_v_x)*MIN_VEL : .999*_v_x;
		cout << std::setprecision(2) << x << ",\t" << y << ",\t" << _v_x << ",\t" << _v_y << endl;
		//cout << _v_x << ", " << _v_y << endl;
	}
	void accel(int a_x, int a_y)
	{
		_a_x = a_x; _a_y = a_y;
		//cout << "accel\n";
	}

	void bringBackToScreen()
	{
		x = width / 3;
		y = height / 3;
		x_ = x;
		y_ = y;
		dstRect.x = x;
		dstRect.y = y;
		_v_x = MIN_VEL;
		_v_y = MIN_VEL;
	}
	bool isOutOfScreen()
	{
		if (x + dstRect.w < 0) return true;
		if (x > width) return true;
		return false;
	}
};

class Paddle : public SDLEntity
{
	SDL_Renderer* _gRenderer = NULL;
	SDL_Texture* _texture = NULL;
	//SDL_Surface* _loadedSurface;


public:
	SDL_Rect srcRect;
	Paddle(string id, SDL_Renderer* gRenderer, string imgpath) :
		SDLEntity(id), _gRenderer(gRenderer)
	{
		SDL_Surface* tempSurface = IMG_Load(imgpath.c_str());
		if (tempSurface == NULL)
		{
			throw runtime_error(string("Failed to load surface") + SDL_GetError() + "\n");
		}
		x = y = 50;
		//_v_y = 2;
		srcRect = { 10 - 1, 8 - 1,11,51 };
		dstRect = { (int)x	,(int)y,11,51 };
		x_ = x;
		y_ = y;

		SDL_SetColorKey(tempSurface, SDL_TRUE, SDL_MapRGB(tempSurface->format, 0xFF, 0xFF, 0xFF));
		//Create texture from surface pixels
		_texture = SDL_CreateTextureFromSurface(_gRenderer, tempSurface);
		SDL_FreeSurface(tempSurface);
		//_v_x = _v_y = 1 * 1.5;
	}
	~Paddle()
	{
		if (NULL != _texture)
			SDL_DestroyTexture(_texture);
	}
	void draw()
	{
		SDL_RenderCopy(_gRenderer, _texture, &srcRect, &dstRect);
	}
	const float MAX_VEL = 300 / pollrate;
	const float MIN_VEL = 1 / pollrate;
	void update()
	{
		x_ = x;
		y_ = y;

		// Deccelerate
		_v_x = (_v_x < -MIN_VEL || _v_x > MIN_VEL) ? .9*_v_x : 0;
		_v_y = (_v_y < -MIN_VEL || _v_y > MIN_VEL) ? .9*_v_y : 0;

		// maybe div by poll rate
		_v_x += _a_x / pollrate; _v_x = (_v_x < -MAX_VEL) ? -MAX_VEL : (_v_x > MAX_VEL) ? MAX_VEL : _v_x;
		_v_y += _a_y / pollrate; _v_y = (_v_y < -MAX_VEL) ? -MAX_VEL : (_v_y > MAX_VEL) ? MAX_VEL : _v_y;

		// Update location
		x += _v_x;
		y += _v_y;
		//cout << rect.x << ", " << rect.y << endl;

		if (y < 0) if(_v_y <0)  _v_y = 0;
		if (y + dstRect.h > height) if (_v_y > 0) _v_y = 0;
		if (y < 0) y = 0;
		if (y + dstRect.h > height) y = height - dstRect.h;

		dstRect.x = x;
		dstRect.y = y;
	}
	void accel(int a_x, int a_y)
	{
		_a_x = a_x; _a_y = a_y;
		//cout << "accel\n";
	}
};


class AIPaddle : public Paddle
{
public:
	AIPaddle(string id, SDL_Renderer* gRenderer, string imgpath) :
		Paddle(id, gRenderer, imgpath)
	{
		x = width - 50;
		y = 50;
		srcRect = { 11 - 1, 69 - 1,11,51 };
		dstRect = { (int)x	,(int)y,11,51 };
		_v_y = 0;
	}
	const int ACCEL = 60;
	void update(vector<Ball> &balls)
	{
		x_ = x;
		y_ = y;
		accel(0, 0);
		Ball* nearest_ball = nullptr;
		int ball_max_x = 0;
		for (auto &b : balls)
		{
			if (b.x < x && ball_max_x < b.x)
			{
				ball_max_x = b.x;
				nearest_ball = &b;
			}
		}
		if (nearest_ball == nullptr) return;

		Ball& b = *nearest_ball;

		{
			// could replace with PID control
			static float e2 = 0;
			static float e1 = 0;
			static float d_e = 0;
			static float i_e = 0;
			static float P = 2;
			static float P_I = .0;
			static float P_D = .5;

			e2 = b.y + (b.dstRect.h/2) - y - (dstRect.h / 2);
			d_e = e2 - e1;
			i_e += e2 ;
			float f_e = P*e2 + P_I*i_e + P_D*d_e;
			accel(0,f_e );

			e1 = e2;

			//cout << b.y << ",\t" << y + (dstRect.h / 2) << ":";
			//if (b.y > y)
			//{
			//	accel(0, ACCEL); cout << "+\n";
			//}
			//if (b.y < y + dstRect.h)
			//{
			//	accel(0, -ACCEL); cout << "-\n";
			//}
		}

		// maybe div by poll rate
		_v_x += _a_x / pollrate; _v_x = (_v_x < -MAX_VEL) ? -MAX_VEL : (_v_x > MAX_VEL) ? MAX_VEL : _v_x;
		_v_y += _a_y / pollrate; _v_y = (_v_y < -MAX_VEL) ? -MAX_VEL : (_v_y > MAX_VEL) ? MAX_VEL : _v_y;

		// Update location
		x += _v_x;
		y += _v_y;
		//cout << rect.x << ", " << rect.y << endl;

		if (x < 0 || x + dstRect.w > width) _v_x = -_v_x;
		if (y < 0 || y + dstRect.h > height) _v_y = -_v_y;
		if (y < 0) y = 0;
		if (y + dstRect.h > height) y = height - dstRect.h;
		dstRect.x = x;
		dstRect.y = y;
	}
};



// Add illusion of depth by having some objects move in the background
class BackgroundParallax : public SDLEntity
{
	SDL_Renderer* _gRenderer = NULL;
	SDL_Texture* _texture = NULL;

	SDL_Rect srcRect1;
	SDL_Rect srcRect2;
	SDL_Rect srcRect3;
	SDL_Rect srcRect4;

	vector<SDL_Rect> dstRects1;
	vector<SDL_Rect> dstRects2;
	vector<SDL_Rect> dstRects3;
	vector<SDL_Rect> dstRects4;
	float y1;
	float y2;
	float y3;
	float y4;

public:
	BackgroundParallax(string id, SDL_Renderer* gRenderer, string imgpath) :
		SDLEntity(id), _gRenderer(gRenderer)
	{
		SDL_Surface* tempSurface = IMG_Load(imgpath.c_str());
		if (tempSurface == NULL)
		{
			throw runtime_error(string("Failed to load surface") + SDL_GetError() + "\n");
		}

		x = uniform_dist_x(e1);
		y1 = uniform_dist_y(e1);
		srcRect1 = { 166 - 1	,8 - 1,186,68 }; srcRect1.w -= srcRect1.x; srcRect1.h -= srcRect1.y;
		for (int i = 0; i < 6; ++i)
		{
			dstRects1.push_back(SDL_Rect{ (int)x + i * 120*2/3 ,(int)y1,srcRect1.w,srcRect1.h });
		}

		x = uniform_dist_x(e1);
		y2 = uniform_dist_y(e1);
		srcRect2 = { 131 - 1	,8 - 1,161 ,98}; srcRect2.w -= srcRect2.x; srcRect2.h -= srcRect2.y;
		for (int i = 0; i < 5; ++i)
		{
			dstRects2.push_back(SDL_Rect{ (int)x + i * 180 * 2 *2/3 ,(int)y2,srcRect2.w ,srcRect2.h  });
		}

		x = uniform_dist_x(e1);
		y3 = uniform_dist_y(e1);
		srcRect3 = { 81 - 1	,8 - 1,126 ,143 }; srcRect3.w -= srcRect3.x; srcRect3.h -= srcRect3.y;
		for (int i = 0; i < 3; ++i)
		{
			dstRects3.push_back(SDL_Rect{ (int)x + i * 270 * 2 /3 ,(int)y3,srcRect3.w ,srcRect3.h  });
		}

		x = uniform_dist_x(e1);
		y4 = uniform_dist_y(e1);
		srcRect4 = { 8 - 1	,8 - 1,76 ,211 }; srcRect4.w -= srcRect4.x; srcRect4.h -= srcRect4.y;
		for (int i = 0; i < 2; ++i)
		{
			dstRects4.push_back(SDL_Rect{ (int)x + i * 405 * 2 /3 ,(int)y4,srcRect4.w ,srcRect4.h  });
		}

		//Create texture from surface pixels
		_texture = SDL_CreateTextureFromSurface(_gRenderer, tempSurface);
		SDL_FreeSurface(tempSurface);
	}
	~BackgroundParallax()
	{
		if (NULL != _texture)
			SDL_DestroyTexture(_texture);
	}
	void draw()
	{
		for (auto r : dstRects1)
		{
			SDL_RenderCopy(_gRenderer, _texture, &srcRect1, &r);
		}
		for (auto r : dstRects2)
		{
			SDL_RenderCopy(_gRenderer, _texture, &srcRect2, &r);
		}
		for (auto r : dstRects3)
		{
			SDL_RenderCopy(_gRenderer, _texture, &srcRect2, &r);
		}
		for (auto r : dstRects4)
		{
			SDL_RenderCopy(_gRenderer, _texture, &srcRect2, &r);
		}
	}
	void update(Paddle &p)
	{

		//cout << "_v_y: " << p.get_v_y() << endl;
		//static int n = 0;
		//if (++n == 200)
		//{
		//	cout << "...\n";
		//}

		y1 -= .1*p.get_v_y();
		y2 -= .3*p.get_v_y();
		y3 -= .6*p.get_v_y();
		y4 -= .9*p.get_v_y();
		for (auto &r : dstRects1)
		{
			r.y = static_cast<int>(y1);
		}
		for (auto &r : dstRects2)
		{
			r.y = static_cast<int>(y2);
		}
		for (auto &r : dstRects3)
		{
			r.y = static_cast<int>(y3);
		}
		for (auto &r : dstRects4)
		{
			r.y = static_cast<int>(y4);
		}

	}
};

 bitset<5> detectCollision(SDLEntity &ref, SDLEntity &obj)
{
	// Using velocity will probably make the collision detection more accurate

	// leftmost bit indicates collision
	bitset<5> res = 0;

	// DEBUG CODE WHEN BALL HITS PADDLE
	//if (ref.get_x() + ref.dstRect.w > 350)
	//{
	//	cout << "Action!\n";
	//}
	//if (ref.get_x() < 362)
	//{
	//	cout << "Action!\n";
	//}
	
	if (ref._v_x>obj._v_x)
		if (ref.get_x() + ref.dstRect.w > obj.get_x())  // ref right bound is more than obj left bound
			if (ref.get_x_() + ref.dstRect.w < obj.get_x())  // ref right bound is previously less than obj left bound 
				if (ref.get_y() + ref.dstRect.h > obj.get_y())  // ref lower bound is lower than obj upper bound
					if (ref.get_y() < obj.get_y() + obj.dstRect.h) // ref upper bound is higher than obj lower bound
						res.set(Collision::Right);

	if (ref._v_x<obj._v_x)
		if (ref.get_x() < obj.get_x() + obj.dstRect.w)  // ref left bound is less than obj right bound
			if (ref.get_x_() > obj.get_x() + obj.dstRect.w)  // ref left bound is previously more than obj right bound 
				if (ref.get_y() + ref.dstRect.h > obj.get_y())  // ref lower bound is lower than obj upper bound
					if (ref.get_y() < obj.get_y() + obj.dstRect.h) // ref upper bound is higher than obj lower bound
						res.set(Collision::Left);

	if (ref._v_y>obj._v_y)
		if (ref.get_y() + ref.dstRect.h > obj.get_y()) // ref lower bound is lower than obj upper bound
			if (ref.get_y_() + ref.dstRect.h < obj.get_y()) // ref lower bound is previously higher than obj upper bound 
				if (ref.get_x() + ref.dstRect.w > obj.get_x()) // ref right bound is more than obj left bound
					if (ref.get_x() < obj.get_x() + obj.dstRect.w) // ref left bound is less than obj right bound
						res.set(Collision::Down);

	if (ref._v_y<obj._v_y)
		if (ref.get_y() < obj.get_y() + obj.dstRect.h)  // ref upper bound is higher than obj lower bound
			if (ref.get_y_() > obj.get_y() + obj.dstRect.h)  // ref upper bound is previously lower than obj lower bound 
				if (ref.get_x() + ref.dstRect.w > obj.get_x())  // ref right bound is more than obj left bound
					if (ref.get_x() < obj.get_x() + obj.dstRect.w) // ref left bound is higher than obj right bound
						res.set(Collision::Up);

	if (res.count() > 0) res.set(Collision::Collided);
	return res;
}


 void handleCollision(SDLEntity &ref, SDLEntity &obj)
 {
	 static bool collided = false;
	 bitset<5> res = detectCollision(ref, obj);
	 if (!collided && res.test(Collision::Collided))
	 {
		 collided = true;

		 if (res.test(Collision::Up))
		 {
			 ref._v_y += 0.5*obj._v_y;
			 ref._v_y = abs(ref._v_y);
		 }
		 if (res.test(Collision::Down))
		 {
			 ref._v_y += 0.5*obj._v_y;
			 ref._v_y = -abs(ref._v_y);
		 }
		 if (res.test(Collision::Left))
		 {
			 ref._v_y += 0.1*obj._v_y;
			 ref._v_x = -1.5*ref._v_x;
		 }
		 if (res.test(Collision::Right))
		 {
			 ref._v_y += 0.1*obj._v_y;
			 ref._v_x = -1.5*ref._v_x;
		 }
	 }
	 else
	 {
		 //cout << "Collision: None\n";
		 collided = false;
	 }

 }
enum GameState {
	Intro, Menu, Playing, GameOver, HighScore, Exit
};
#include <typeinfo>
int main(int argc, char* argv[])
{
	SDLEventManager eventManager(pollrate, width, height);
	SDLDisplayManager displayManager(width, height, framerate, eventManager.getWindow());

	GameState gameState = GameState::Menu;
	int level = 1;
	
	// Load games resources
	Background bg("Background", displayManager.getScreenRenderer(), "../sprites/bg02.png");
	BackgroundParallax bg_parallax("Background Parallax", displayManager.getScreenRenderer(), "../sprites/sprites02.png");
	Paddle paddle("Paddle", displayManager.getScreenRenderer(), "../sprites/pong.png");
	AIPaddle aipaddle("Paddle", displayManager.getScreenRenderer(), "../sprites/pong.png");
	
	vector<Ball> balls;
	const int n_balls = 1;
	balls.reserve(n_balls);
	//Ball ball("Critter", displayManager.getScreenRenderer(), "../sprites/pong.png");



	// Register drawing
	displayManager.registerEntity(bind(&Background::draw, &bg));
	displayManager.registerEntity(bind(&BackgroundParallax::draw, &bg_parallax));
	//displayManager.registerEntity(bind(&Ball::draw, &ball));
	displayManager.registerEntity(bind(&Paddle::draw, &paddle));
	//displayManager.registerEntity(bind(&AIPaddle::draw, &aipaddle));
	displayManager.registerEntity(bind(&decltype(aipaddle)::draw, &aipaddle));

	SDLEntity& a = aipaddle;
	cout << "Class: " << typeid(a).name() << "\n";

	// Need to register key up also
	eventManager.registerKeyHandler(
	// KeyDownHandler	
	[&](SDL_Keycode k)
	{
		const float ACCEL = 50;
		switch (k)
		{
		case SDLK_UP:
			paddle.accel(0, -ACCEL); 
			break;
		case SDLK_DOWN:
			paddle.accel(0, ACCEL); 
			break;
		default:
			break;
		}

	},
	// KeyUpHandler
	[&](SDL_Keycode k)
	{
		switch (k)
		{
		case SDLK_UP:
			paddle.accel(0, 0);
			break;
		case SDLK_DOWN:
			paddle.accel(0, 0);
			break;
		default:
			break;
		}

	});

	for (int i = 0; i < n_balls; ++i)
	{
		balls.emplace_back(move(Ball("Ball", displayManager.getScreenRenderer(), "../sprites/pong.png")));
	}
	for (auto &b : balls)
	{
		displayManager.registerEntity(bind(&Ball::draw, &b));
		b.bringBackToScreen();
	}

	// Game loop
	chrono::high_resolution_clock::time_point t1;
	chrono::high_resolution_clock::time_point t2;

	t1 = chrono::high_resolution_clock::now();
	while (1)
	{
		switch (gameState)
		{
		case GameState::Intro:
		{
			IntroScreen intro("Intro screen", displayManager.getScreenRenderer(), "../sprites/intro.png");
			cout << "Intro start\n";
			for (int n = 0; n < 6; ++n)
			{
				intro.draw();
				intro.update();
				SDL_RenderPresent(displayManager.getScreenRenderer());
				std::this_thread::sleep_for(chrono::milliseconds(150));
			}
			std::this_thread::sleep_for(chrono::milliseconds(2000));
			cout << "Intro end\n";
			break;
		}
		case GameState::Menu:
			cout << "Game Menu\n";

			for (auto &b : balls)
			{
				b.bringBackToScreen();
			}

			gameState = GameState::Playing;
			displayManager.startDrawing();
			break;
		case GameState::Playing:
			cout << "Game Playing\n";
			for (auto &b : balls)
			{
				handleCollision(b, paddle);
				handleCollision(b, aipaddle);
				b.update();
			}
			// has the ball been lost?
			//{
			//	
			//	auto new_end = std::remove_if(balls.begin(), balls.end(), std::bind(&Ball::isOutOfScreen, std::placeholders::_1));
			//	balls.erase(new_end, balls.end()); // still need to set the balls.end
			//}

			paddle.update();
			aipaddle.update(balls);
			bg_parallax.update(paddle);
			{
				int ballsOnScreen = balls.size() - std::count_if(balls.begin(), balls.end(), std::bind(&Ball::isOutOfScreen, std::placeholders::_1));

				if (ballsOnScreen == 0)
				{
					// let player come to terms with missing balls (cue awkward silence...)
					std::this_thread::sleep_for(chrono::milliseconds(1000));
					gameState = GameState::GameOver;
				}

			}
			break;
		case GameState::GameOver:
			cout << "Game Over\n";
			{
				displayManager.stopDrawing();
				GameOverScreen gameover("Game over screen", displayManager.getScreenRenderer(), "../sprites/game_over.png");
				gameover.draw();
				SDL_RenderPresent(displayManager.getScreenRenderer());
				std::this_thread::sleep_for(chrono::seconds(3));
				gameState = GameState::Menu;
			}
			break;
		case GameState::HighScore:
			cout << "High Score\n";
			break;
		case GameState::Exit:
			cout << "Exit\n";
			break;
		default:
			break;
		}


		t2 = chrono::high_resolution_clock::now();
		this_thread::sleep_for(chrono::milliseconds(1000 / pollrate) - (t2 - t1));
		t1 = t2;
	}
	
    return 0;
}

