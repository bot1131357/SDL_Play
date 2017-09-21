#pragma once

#include <iostream>

using namespace std;
/**
* Where configuration lives
* May be used for loading a config file in future
*/
class GameConfig
{
	GameConfig() // private constructor prevents user from constructing
	{
		std::cout << "Class instantiated!\n";
	}
	~GameConfig()
	{
		std::cout << "Destruction.\n";
	}
	// declaring private copy constructor and assignment operator prevents user from making copies
	// copy
	GameConfig(GameConfig const&) = delete; // copy
	GameConfig& operator=(GameConfig const&) = delete; // assign
	GameConfig(GameConfig &&) = delete; // move
	GameConfig& operator=(GameConfig const&&) = delete; // assign

public:

	static GameConfig& getInstance()
	{
		static GameConfig singleton;
		return singleton;
	}

	// Config values
	const int framerate = 30;
	const int pollrate = 60;
	const int window_width = 400;
	const int window_height = 300;
	bool quit = false;
};