#include "display.h"

#ifdef TEST_DISPLAY
#include "font.h"
#endif

Display::Display(int width, int height, int unit) :
    width(width), height(height), unit(unit),
    renderer(NULL), screen(height, std::vector<bool>(width, false)) {
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
  for (int r = 0; r < height; ++r) {
      for (int c = 0; c < width; ++c) {
          screen[r][c] = false;
      }
  }
}

bool Display::draw(Sprite sprite, int x, int y) {
    bool collision = false;
    for (int r = 0; r < sprite.size(); ++r) {
        uint8_t columnMask = sprite[r];
        for (int c = 0; c < 8; ++c) {
            bool filled = (columnMask >> (7-c)) & 1;
            int _r = (y+r)%height, _c = (x+c)%width;
            bool newState = filled ^ screen[_r][_c];
            if (!newState && screen[_r][_c]) {
                collision = true;
            }
            bool change = screen[_r][_c] != newState;
            screen[_r][_c] = newState;

            if (change) {
                SDL_Rect rect;
                rect.x = _c*unit;
                rect.y = _r*unit;
                rect.w = unit;
                rect.h = unit;
                if (newState) {
                    SDL_SetRenderDrawColor(renderer, 0xE0, 0x00, 0x00, 0xFF);
                } else {
                    SDL_SetRenderDrawColor(renderer, 0x01, 0x01, 0x01, 0xFF);
                }
                SDL_RenderFillRect(renderer,&rect);
            }
        }
    }
    return collision;
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
                mask |= (1<<(7-c));
            }
        }
        ret.push_back(mask);
    }
    return ret;
}

#ifdef TEST_DISPLAY
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
    d.draw(makeSprite(
                      "10010001"
                      "00101000"
                      "00101000"
                      "00010000"
                      "00010000"
                      "00101000"
                      "00101000"
                      "10010001"), 50, 15);
    d.render();
    SDL_Delay( 2000 );
    d.clear();

    for (int i = 0; i < 16; ++i) {
        d.draw(makeSprite(chip8_fonts[i]), i*5,i*5);
        d.render();
        SDL_Delay(200);
    }

    for (int i = 0; i < 16; ++i) {
        d.draw(makeSprite(chip8_fonts[i]), i*5,i*5);
        d.render();
        SDL_Delay(200);
    }

    SDL_Quit();
}

int main() {
    testDisplay();
    return 0;
}
#endif
