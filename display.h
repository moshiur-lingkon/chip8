#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <vector>
#include <stdint.h>
#include <SDL2/SDL.h>

typedef std::vector<uint8_t> Sprite;

class Display {
    int width;
    int height;
    int unit;
    SDL_Window* window;
    SDL_Renderer* renderer;
public:
    Display(int width, int height, int unit);
    void init();
    void release();
    void clear();
    void render();
    void draw(Sprite sprite, int x, int y);
};

void testDisplay();

#endif
