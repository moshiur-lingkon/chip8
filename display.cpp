#include "display.h"
#include "font.h"

Display::Display(int width, int height, int unit) :
    width(width), height(height), unit(unit), renderer(NULL) {
}

void Display::init() {
    window = SDL_CreateWindow("CHIP-8 Emulator",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                width * unit, height * unit,
                SDL_WINDOW_OPENGL);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void Display::release() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void Display::clear() {
  SDL_SetRenderDrawColor(renderer, 0x01, 0x01, 0x01, 0xFF);
  SDL_RenderClear(renderer);
}

void Display::draw(Sprite sprite, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 0xE0, 0x00, 0x00, 0xFF);
    for (int r = 0; r < sprite.size(); ++r) {
        uint8_t columnMask = sprite[r];
        for (int c = 0; c < 8; ++c) {
            bool fill = (columnMask >> c) & 1;
            if (fill) {
                SDL_Rect rect;
                rect.x = (x+c)*unit;
                rect.y = (y+r)*unit;
                rect.w = unit;
                rect.h = unit;
                SDL_RenderFillRect(renderer,&rect);
            }
        }
    }
}

void Display::render() {
   SDL_RenderPresent(renderer);
}

Sprite makeSprite(const char* str) {
    Sprite ret;
    int len = strlen(str);
    int rows = len/8;
    for (int r = 0; r < rows; ++r) {
        uint8_t mask = 0;
        for (int c = 0; c < 8; ++c) {
            if (str[r*8+c] == '1') {
                mask |= (1<<c);
            }
        }
        ret.push_back(mask);
    }
    return ret;
}

void testDisplay() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("failed to SDL_INIT_EVERYTHING, error %s", SDL_GetError());
    }

    Display d(100,90,8);
    d.init();
    d.clear();
    d.draw(makeSprite(
                      "00100000"
                      "00110000"
                      "00111100"
                      "00000100"
                      "00000100"), 10, 10);

    d.draw(makeSprite(
                      "01010101"
                      "10101010"
                      "01010101"
                      "10101010"
                      "01010101"
                      "10101010"
                      "01010101"
                      "10101010"
                      ), 40, 15);
    d.render();
    SDL_Delay( 2000 );
    SDL_Log("Phase 2");
    d.clear();
    d.draw(makeSprite(
                      "00010000"
                      "00101000"
                      "00101000"
                      "00010000"
                      "00010000"
                      "00101000"
                      "00101000"
                      "00010000"), 50, 15);
    d.render();
    SDL_Delay( 2000 );
    d.clear();

    for (int i = 0; i < 16; ++i) {
        d.draw(makeSprite(fonts[i]), i*5,i*5);
        d.render();
        SDL_Delay(500);
    }

    d.draw(makeSprite(fonts[0xA]), 40,3);
    d.draw(makeSprite(fonts[0x6]), 50,3);
    d.render();
    SDL_Delay(500);

    SDL_Quit();
}

int main() {
    testDisplay();
    return 0;
}
