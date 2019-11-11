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
    std::vector<std::vector<bool> > screen;
public:
    Display(int width, int height, int unit);
    void init();
    void release();
    void clear();
    void render();
    bool draw(Sprite sprite, int x, int y);
};

Sprite makeSprite(const char* str);

void testDisplay();

#endif
