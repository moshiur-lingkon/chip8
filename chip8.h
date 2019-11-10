#ifndef __CHIP8_H__
#define __CHIP8_H__

#include "display.h"

struct Timer {
    uint8_t startVal;
    uint32_t setTimeMs;

    uint8_t val(uint32_t nowMs) {
        uint32_t elapsed = (nowMs - setTimeMs)/1000;
        if (elapsed < startVal) {
            return startVal - elapsed;
        }
        return 0;
    }

    uint8_t set(uint8_t val, uint32_t nowMs) {
        startVal = val;
        setTimeMs = nowMs;
    }
};

struct Registers {
    uint16_t PC;
    uint16_t I;

    uint8_t V[16];

    Timer delayTimer;
    Timer soundTimer;

    uint8_t stackPtr;
};

class Chip8Vm {
    Display display;

    Registers reg;
    uint16_t stack[32];
    uint8_t memory[0x1000];
    uint16_t digitSpritePos[16];

    bool keyDown[16];

    void loadFonts();
    void pollEvents(bool& exit);
    void run();

    uint16_t readOpcode();
    uint8_t randomByte();
    Sprite loadSprite(uint16_t pos, uint8_t bytes);

public:
    Chip8Vm();
    ~Chip8Vm();
    void emulate(const std::vector<uint16_t>& rom);
};

#endif
