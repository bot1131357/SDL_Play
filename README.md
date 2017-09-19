# SDL_Play
## Experiment with SDL. 
Part of my on-going experiments, where I journal the challenges and also my thoughts on what works and what doesn't.

SDL is written in C, and therefore it gets a little challenging to implement C++ style code. This explains why some of the code gets a little warped. Threading is also an enigma. SDL_Init(), SDL_CreateWindow() and SDL_PollEvent() had to be within the same thread, or the events won't happen. Who knows what other gotchas will I encounter down the road...

Most of what I've learned by copying code from [Lazy Foo' Production](http://lazyfoo.net/SDL_tutorials/)

Some of the ideas which I'd try to make some simple games or... just random things.

## 01_GameEngine_v2
My implementation of a super minimal pong

### Use of scenes
It's easy to register and unregister entities to a list of items that needs to be drawn on the screen, but if you want to manage more things, or have a set of things to be shown on different times, managing them as scenes could be useful. 

Scene instances can also be in charged of storing textures. This way, identical entities (multiple balls) don't have to store duplicate textures on their own.

But how does the scene know which texture it should use for a given entity?



Scene scene_Gameplay;




In the case of pong, you may want the paddles and ball to be hidden during intro, menu or game over. Implementing scenes might be the way to go:

```C++

Scene scene_Intro;
Scene scene_Menu;
Scene scene_Gameplay;
Scene scene_GameOver;

scene_Intro.add(intro_bg);

scene_Menu.add(menu_bg);
scene_Menu.add(menu_options);
scene_Menu.add(menu_highlightedOption);

scene_Gameplay.add(game_bg);
scene_Gameplay.add(game_bgElements); // interactive bg elements
scene_Gameplay.add(game_paddle);
scene_Gameplay.add(game_paddle_ai);
scene_Gameplay.add(game_balls);
scene_Gameplay.add(game_text);

scene_GameOver.add(gameover_bg);
scene_GameOver.add(gameover_text);


...

// during intro
sceneManager.play(scene_Intro);

// during gameplay
sceneManager.play(scene_Gameplay);

...

```


## Draw this ball...no wait... the ball is gone. Stop drawing it.
Registering the draw() method for every entity inside a vector is ok if you don't create and destroy entities. If you need to register and unregister, a hash map might be more useful. Something to keep in mind when I trash the whole program to restart anew.






# Other
This stays here until I get the hang of github formatting 
[Formatting guide](https://help.github.com/articles/basic-writing-and-formatting-syntax/)



