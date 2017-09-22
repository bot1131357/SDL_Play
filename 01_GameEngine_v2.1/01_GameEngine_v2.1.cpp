// 01_GameEngine_v2.1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <iomanip> 
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <bitset>
#include <algorithm>
#include "SDL.h"
#include "SDL_image.h"

#include "GameConfig.h"
#include "Entity.h"
using namespace std;

/**
 * 
 */
enum GameState {
	Intro, Menu, Playing, GameOver, HighScore, Exit
};




class SDLEventManager
{
	atomic<bool> _keepRunning;
	thread _eventLoop;
	atomic<SDL_Window*> _gWindow;
	SDL_Surface* _gScreenSurface = NULL;

	function<void(SDL_Event)> _eventHandler;
	function<void(SDL_Keycode)> _keyDownHandler;
	function<void(SDL_Keycode)> _keyUpHandler;

public:
	SDLEventManager() :
	 _keepRunning(true), _gWindow(NULL)
	{
		GameConfig& gc = GameConfig::getInstance();
		
		_keyDownHandler = bind([](SDL_Keycode k) {}, placeholders::_1);
		_keyUpHandler = bind([](SDL_Keycode k) {}, placeholders::_1);
		_eventHandler = bind([](SDL_Event e) {cout << "Event\n"; }, placeholders::_1);

		_eventLoop = thread([&]() {

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
			_gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gc.window_width, gc.window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
			if (_gWindow.load() == NULL)
			{
				throw runtime_error(string("Failed to create Window") + SDL_GetError() + "\n");
			}

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
						if (keycode != e.key.keysym.sym || t2 - t1 > chrono::milliseconds(1000 / gc.pollrate))
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
						gc.quit = true;
					default:
						_eventHandler(e);
					}
				}
			}
		});
	}
	~SDLEventManager() {
		_keepRunning = false;
		if (_eventLoop.joinable())_eventLoop.join();
	}
	SDL_Window* getWindow() {
		while (NULL == _gWindow.load())
		{
			this_thread::sleep_for(chrono::milliseconds(10));
		}
		return _gWindow.load();
	}

	void registerEventHandler(function<void(SDL_Event)> eventHandler)
	{
		_eventHandler = eventHandler;
	}

	void registerKeyHandler(function<void(SDL_Keycode)> keyDownHandler, function<void(SDL_Keycode)> keyUpHandler)
	{
		_keyDownHandler = keyDownHandler;
		_keyUpHandler = keyUpHandler;
	}

	void registerMouseHandler(function<void(SDL_Event)> inputHandler)
	{
		// stub
	}
};

class SDLViewManager
{
	std::mutex mtx_drawing;
	std::condition_variable cv_drawing;
	bool keepDrawing = false;

	mutex mtx_drawfunctions;
	bool _keepAlive;
	thread _thread_draw;
	function<void(void)> _drawFunction = []() {};
	unordered_map<string, SDL_Texture*> _textures;

	SDL_Window* _window = NULL;
	SDL_Renderer* _renderer = NULL;
	SDL_Surface* _screenSurface = NULL;
public:
	SDLViewManager(SDL_Window* gWindow) :
		_window(gWindow), _keepAlive(true)
	{
		//Get window surface
		_screenSurface = SDL_GetWindowSurface(_window);

		//Create renderer for window
		_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

		//Initialize renderer color
		SDL_SetRenderDrawColor(_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	}

	~SDLViewManager() {
		_keepAlive = false;
		if (_thread_draw.joinable()) _thread_draw.join();
	}

	void addTexture(string id, string imgpath)
	{
		SDL_Surface* tempSurface = IMG_Load(imgpath.c_str());
		if (tempSurface == NULL) { throw runtime_error(string("Failed to load surface") + SDL_GetError() + "\n"); }
		SDL_SetColorKey(tempSurface, SDL_TRUE, SDL_MapRGB(tempSurface->format, 0xFF, 0xFF, 0xFF));
		SDL_Texture* _texture = SDL_CreateTextureFromSurface(_renderer, tempSurface);
		SDL_FreeSurface(tempSurface);

		// Do I need to check for null texture?
		_textures[id] = _texture;
	}

	void startDrawing()
	{
		_keepAlive = true;
		_thread_draw = thread([=]() {
			GameConfig& gc = GameConfig::getInstance();
			chrono::high_resolution_clock::time_point t1;
			chrono::high_resolution_clock::time_point t2;
			t1 = chrono::high_resolution_clock::now();
			while (_keepAlive)
			{
				// Calls the registered draw functions
				{
					lock_guard<mutex> lg(mtx_drawfunctions);
					_drawFunction();
				}

				// Update rendered screen
				SDL_RenderPresent(_renderer);

				t2 = chrono::high_resolution_clock::now();
				this_thread::sleep_for(chrono::milliseconds(1000 / gc.framerate) - (t2 - t1));
				t1 = t2;
			}
		});
	}
	void stopDrawing() 
	{
		_keepAlive = false;
		_thread_draw.join();
	}
	const SDL_Surface* getScreenSurface() { return _screenSurface; }
	unordered_map<string, SDL_Texture*>& getTextures() { return _textures; }
	SDL_Renderer* getScreenRenderer() { return _renderer; }
	void setDrawFunction(function<void(void)> drawfunction) {
		lock_guard<mutex> lg(mtx_drawfunctions);
		_drawFunction = drawfunction;
	}


};


//SDL_Texture* loadTexture(SDL_Renderer* renderer , string imgpath)
//{
//	SDL_Surface* tempSurface = IMG_Load(imgpath.c_str());
//	if (tempSurface == NULL)
//	{
//		throw runtime_error(string("Failed to load surface") + SDL_GetError() + "\n");
//	}
//	SDL_SetColorKey(tempSurface, SDL_TRUE, SDL_MapRGB(tempSurface->format, 0xFF, 0xFF, 0xFF));
//	SDL_Texture* _texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
//	SDL_FreeSurface(tempSurface);
//	return _texture;
//}
class Scene
{
protected:
	bool keepAlive = true;
	unordered_map<string, SDL_Texture*> _textures;
	vector<unordered_set<shared_ptr<Entity>>> _layers;
	SDL_Renderer* _renderer;
	function<void(SDL_Keycode)> _keyUpHandler = [&](SDL_Keycode k) {};
	function<void(SDL_Keycode)> _keyDownHandler = [&](SDL_Keycode k) {};
public:

	Scene(SDLViewManager& viewMan) :_renderer(viewMan.getScreenRenderer()), _textures(viewMan.getTextures()) { viewMan.setDrawFunction(bind(&Scene::drawFunction, this)); }

	void drawFunction()
	{
		for (auto& m : _layers)
		{
			for (auto& e : m)
			{
				// draw 
				SDL_RenderCopy(_renderer, _textures[e->get_id()], &(e->get_srcRect()), &(e->get_dstRect()));
				e->animate();
			}
		}
	}
};

class SceneIntro : public Scene
{

public:
	SceneIntro(SDLViewManager& viewMan) :Scene(viewMan)
	{
		_keyUpHandler = [&](SDL_Keycode k) {keepAlive = false; };

	}
	void run()
	{
		GameConfig& gc = GameConfig::getInstance();
		chrono::high_resolution_clock::time_point t1;
		chrono::high_resolution_clock::time_point t2;
		t1 = chrono::high_resolution_clock::now();

		while (keepAlive)
		{
			for (auto& m : _layers)
			{
				for (auto& e : m)
				{
					// update
					e->update();
				}
			}

		}
		// Update rendered screen
		SDL_RenderPresent(_renderer);

		t2 = chrono::high_resolution_clock::now();
		this_thread::sleep_for(chrono::milliseconds(1000 / gc.framerate) - (t2 - t1));
		t1 = t2;
	}
	void addEntity(shared_ptr<Entity> e, uint8_t l)
	{
		for (int i = _layers.size(); i < l + 1; ++i)
		{
			_layers.emplace_back(unordered_set<shared_ptr<Entity>>());
		}
		_layers[l].insert(e);
	}

};
class SceneMenu {
public:
	SceneMenu()
	{
		while (1)
			this_thread::sleep_for(chrono::milliseconds(100));
	}
};
class ScenePlaying {};
class SceneGameOver {};
class SceneHighScore {};


int main(int argc, char* argv[])
{
	GameConfig& gc = GameConfig::getInstance();

	SDLEventManager eventManager;

	GameState gameState = GameState::Intro;

	SDLViewManager viewMan(eventManager.getWindow());
	viewMan.addTexture("intro", "../sprites/intro.png");
	viewMan.startDrawing();

	// Load resources
	//SDL_Texture* intro = loadTexture(viewMan.getScreenRenderer(), "../sprites/intro.png");



	// Game loop
	while (!gc.quit)
	{
		switch (gameState)
		{
		case GameState::Intro:
			cout << "Intro start\n";
			{
				SDL_Rect srcRect{ 0,0,640,480 };
				shared_ptr<Entity> intro(new AnimateFrames("intro", 6, srcRect, FrameOrientation::Down));
				SceneIntro scene(viewMan);
				scene.addEntity(intro,0);
				viewMan.setDrawFunction(bind(&SceneIntro::drawFunction, &scene));
				scene.run();
			}
			cout << "Intro end\n";
			break;
		case GameState::Menu:
			cout << "Menu start\n";
			{
				SceneMenu menu();
			}
			cout << "Menu end\n";
			break;
		case GameState::Playing:
			cout << "Playing start\n";
			{
				ScenePlaying playing();
			}
			cout << "Playing end\n";
			break;
		case GameState::GameOver:
			cout << "GameOver start\n";
			{
				SceneGameOver gameOver();
			}
			cout << "GameOver end\n";
			break;
		case GameState::HighScore:
			cout << "GameOver start\n";
			{
				SceneHighScore highscore();
			}
			cout << "GameOver end\n";
			break;
		case GameState::Exit:
			cout << "Exit\n";
			break;
		default:
			break;
		}
	}

	return 0;
}

