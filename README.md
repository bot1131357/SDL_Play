# SDL_Play
Experiment with SDL. 
SDL is written in C, and therefore it gets a little challenging to implement C++ style code. This explains why some of the code gets a little warped. Threading is also an enigma. SDL_Init(), SDL_CreateWindow() and SDL_PollEvent() had to be within the same thread, or the events won't happen. Who knows what other gotchas will I encounter down the road...

Most of what I've learned by copying code from [Lazy Foo' Production](http://lazyfoo.net/SDL_tutorials/)

Some of the ideas which I'd try to make some simple games or... just random things.

## 01_GameEngine_v2
My implementation of a super minimal pong


