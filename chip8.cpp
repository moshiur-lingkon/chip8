#include <stdlib.h>
#include "chip8.h"
#include "font.h"

Chip8Vm::Chip8Vm() : display(64,32,10) {
    reg.delayTimer.startVal = 0;
    reg.soundTimer.startVal = 0;
    reg.stackPtr = 0;

    display.init();
    loadFonts();
}

Chip8Vm::~Chip8Vm() {
    display.release();
}

void Chip8Vm::loadFonts() {
    uint16_t pos = 0;
    for (uint8_t digit = 0; digit < 16; ++digit) {
        Sprite s = makeSprite(chip8_fonts[digit]);
        digitSpritePos[digit] = pos;
        for (int i = 0; i < s.size(); ++i) {
            memory[pos++] = s[i];
        }
    }
}

Sprite Chip8Vm::loadSprite(uint16_t pos, uint8_t bytes) {
    Sprite ret;
    for (int i = 0; i < bytes; ++i) {
        ret.push_back(memory[pos+i]);
    }
    return ret;
}

void Chip8Vm::run() {
    uint16_t ins = memory[reg.PC]<<8 | memory[reg.PC+1];
    uint8_t t = (ins>>12);
    uint8_t x = (ins>>8)&0xF;
    uint8_t y = (ins>>4)&0xF;
    uint8_t z = ins & 0xF;
    uint8_t nnn = ins & 0xFFF;
    uint8_t kk = ins & 0xFF;

    switch (t) {
        case 0x0:
            switch (ins) {
                case 0x00E0:
                    display.clear();
                    display.render();
                    reg.PC++;
                    break;
                case 0x00EE:
                    reg.PC = stack[reg.stackPtr--];
                    break;
            }
            break;
        case 0x1: // 0x1nnn
            reg.PC = nnn;
            break;
        case 0x2: // 0x2nnn
            stack[++reg.stackPtr] = reg.PC;
            reg.PC = nnn;
            break;
        case 0x3: // 0x3xkk
            if (reg.V[x] == kk) {
                reg.PC += 2;
            }
            break;
        case 0x4: // 0x4xkk
            if (reg.V[x] != kk) {
                reg.PC += 2;
            }
            break;
        case 0x5: // 0x5xy0 TODO: check if z is 0
            if (reg.V[x] == reg.V[y]) {
                reg.PC += 2;
            }
            break;
        case 0x6: // 0x6xkk
            reg.V[x] = kk;
            reg.PC++;
            break;
        case 0x7: // 0x7xkk
            reg.V[x] += kk;
            reg.PC++;
            break;
        case 0x8: // 0x8xyz
            switch (z) {
                case 0:
                    reg.V[x] = reg.V[y];
                    break;
                case 1:
                    reg.V[x] |= reg.V[y];
                    break;
                case 2:
                    reg.V[x] &= reg.V[y];
                    break;
                case 3:
                    reg.V[x] ^= reg.V[y];
                    break;
                case 4:
                {
                    uint16_t bigVx = reg.V[x];
                    uint16_t bigVy = reg.V[y];
                    uint16_t sum = bigVx + bigVy;
                    reg.V[0xF] = sum > 255 ? 1 : 0;
                    reg.V[x] = sum & 0xFFFF;
                }
                    break;
                case 5:
                    reg.V[0xF] = reg.V[x] > reg.V[y] ? 1 : 0;
                    reg.V[x] -= reg.V[y]; // TODO: we are using unsgined??
                    break;
                case 6:
                    reg.V[0xF] = reg.V[x]&1;
                    reg.V[x] >>= 1;
                    break;
                case 7:
                    reg.V[0xF] = reg.V[y] > reg.V[x] ? 1 : 0;
                    reg.V[x] = reg.V[y] - reg.V[x]; // TODO: we are using unsgined??
                    break;
                case 0xE:
                    reg.V[0xF] = (reg.V[x]>>7)&1;
                    reg.V[x] <<= 1;
                    break;
            }
            reg.PC++;
            break;
        case 0x9: // 0x9xy0
            if (reg.V[x] != reg.V[y]) {
                reg.PC += 2;
            }
            break;
        case 0xA: // 0xAnnn
            reg.I = nnn;
            reg.PC++;
            break;
        case 0xB: // 0xBnnn
            reg.PC = reg.V[0] + nnn;
            break;
        case 0xC: // 0xCxkk
            reg.V[x] = randomByte() & kk;
            reg.PC++;
            break;
        case 0xD: // 0xDxyz
            reg.V[0xF] = display.draw(loadSprite(reg.I, z), reg.V[x], reg.V[y]) ? 1 : 0;
            display.render();
            reg.PC++;
            break;
        case 0xE:
            switch (kk) {
                case 0x9E:
                    if (keyDown[reg.V[x]]) {
                        reg.PC += 2;
                    } else {
                        reg.PC++;
                    }
                    break;
                case 0xA1:
                    if (!keyDown[reg.V[x]]) {
                        reg.PC += 2;
                    } else {
                        reg.PC++;
                    }
                    break;
            }
            break;
        case 0xF:
            switch (kk) {
                case 0x07:
                    reg.V[x] = reg.delayTimer.val(SDL_GetTicks());
                    break;
                case 0x0A:
                {
                    bool isPressed = false;
                    uint8_t pressedKey = 0;
                    for (uint8_t key = 0; key <= 0xF; ++key) {
                        if (keyDown[key]) {
                            pressedKey = key;
                            isPressed = true;
                            break;
                        }
                    }
                    if (isPressed) {
                        reg.V[x] = pressedKey;
                        reg.PC++;
                    }
                }
                    break;
                case 0x15:
                    reg.delayTimer.set(reg.V[x], SDL_GetTicks());
                    reg.PC++;
                    break;
                case 0x18:
                    reg.soundTimer.set(reg.V[x], SDL_GetTicks());
                    reg.PC++;
                    break;
                case 0x1E:
                    reg.I = reg.I + reg.V[x];
                    reg.PC++;
                    break;
                case 0x29:
                    reg.I = digitSpritePos[reg.V[x]];
                    reg.PC++;
                    break;
                case 0x33:
                    {
                        uint8_t d100 = reg.V[x]/100;
                        uint8_t d10  = (reg.V[x]/10)%10;
                        uint8_t d    = reg.V[x]%10;
                        memory[reg.I]   = d100;
                        memory[reg.I+1] = d10;
                        memory[reg.I+2] = d;
                        reg.PC++;
                    }
                    break;
                case 0x55:
                    for (uint8_t r = 0; r <= x; ++r) {
                        memory[reg.I+r] = reg.V[r];
                    }
                    reg.PC++;
                    break;
                case 0x65:
                    for (uint8_t r = 0; r <= x; ++r) {
                        reg.V[r] = memory[reg.I+r];
                    }
                    reg.PC++;
                    break;
            }
            break;
    }
}

void Chip8Vm::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                {
                    int keyCodes[] = {SDLK_0,SDLK_1,SDLK_2,SDLK_3,SDLK_4,SDLK_5,SDLK_6,SDLK_7,SDLK_8,SDLK_9,SDLK_a,SDLK_b,SDLK_c,SDLK_d,SDLK_e,SDLK_f};
                    for (int k = 0; k < 16; ++k) {
                        if (keyCodes[k] == event.key.keysym.sym) {
                            keyDown[k] = (event.type == SDL_KEYDOWN);
                        }
                    }
                }
                break;
            case SDL_WINDOWEVENT_EXPOSED:
                display.render();
                break;
        }
    }
}

void Chip8Vm::emulate(const std::vector<uint16_t>& rom) {
    for (int i = 0; i < rom.size(); ++i) {
        uint16_t ins = rom[i];
        memory[0x200 + i*2] = ins >> 8;
        memory[0x200 + i*2 + 1] = ins & 0xFF;
    }
    reg.PC = 0x200;
    while (true) {
        pollEvents();
        run();
    }
}

uint8_t Chip8Vm::randomByte() {
    return rand() & 0xFF;
};
