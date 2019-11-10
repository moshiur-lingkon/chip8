#include <stdlib.h>
#include <string.h>
#include "chip8.h"
#include "font.h"

Chip8Vm::Chip8Vm() : display(64,32,20) {
    reg.delayTimer.startVal = 0;
    reg.soundTimer.startVal = 0;
    reg.stackSize = 0;

    memset(keyDown, 0, sizeof(keyDown));

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
    printf("loadSprite: %d, %d\n", pos, bytes);
    Sprite ret;
    for (int i = 0; i < bytes; ++i) {
        ret.push_back(memory[pos+i]);
        char bin[9];bin[9]=0;
        for (int j=0;j<8;++j){
            bin[j]='0' + ((memory[pos+i]>>j)&1);
        }
        printf("%s\n",bin);
    }
    return ret;
}

uint16_t Chip8Vm::readOpcode() {
    uint16_t opcode = (memory[reg.PC] << 8) | 
                       memory[reg.PC + 1];
    return opcode;
}

void Chip8Vm::run() {
    uint16_t opcode = readOpcode();
    uint8_t t = (opcode>>12);
    uint8_t x = (opcode>>8)&0xF;
    uint8_t y = (opcode>>4)&0xF;
    uint8_t z = opcode & 0xF;
    uint16_t nnn = opcode & 0xFFF;
    uint8_t kk = opcode & 0xFF;

    printf("Registers: PC => %d, I => %d\n", reg.PC, reg.I);
    printf("opcode[%x]: %x %x %x %x, nnn %x, kk %x\n", opcode, t, x, y, z, nnn, kk);
    printf("-------------\n");

    auto startTime = std::chrono::steady_clock::now();
    bool PCModified = false;

    switch (t) {
        case 0x0:
            switch (opcode) {
                case 0x00E0:
                    display.clear();
                    display.render();
                    break;
                case 0x00EE:
                    reg.PC = stack[--reg.stackSize];
                    PCModified = true;
                    break;
                default:
                    printf(">>>>>>>>>>>>>>>>> ERROR!!!\n");
                    break;
            }
            break;
        case 0x1: // 0x1nnn
            reg.PC = nnn;
            PCModified = true;
            break;
        case 0x2: // 0x2nnn
            stack[reg.stackSize++] = reg.PC;
            reg.PC = nnn;
            PCModified = true;
            break;
        case 0x3: // 0x3xkk
            if (reg.V[x] == kk) {
                reg.PC += 4;
                PCModified = true;
            }
            break;
        case 0x4: // 0x4xkk
            if (reg.V[x] != kk) {
                reg.PC += 4;
                PCModified = true;
            }
            break;
        case 0x5: // 0x5xy0 TODO: check if z is 0
            if (reg.V[x] == reg.V[y]) {
                reg.PC += 4;
                PCModified = true;
            }
            break;
        case 0x6: // 0x6xkk
            reg.V[x] = kk;
            break;
        case 0x7: // 0x7xkk
            reg.V[x] += kk;
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
                    reg.V[x] = sum & 0xFF;
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
                default:
                    printf(">>>>>>>>>>>>>>>>> ERROR!!!\n");
                    break;
            }
            break;
        case 0x9: // 0x9xy0
            if (reg.V[x] != reg.V[y]) {
                reg.PC += 4;
                PCModified = true;
            }
            break;
        case 0xA: // 0xAnnn
            reg.I = nnn;
            break;
        case 0xB: // 0xBnnn
            reg.PC = reg.V[0] + nnn;
            break;
        case 0xC: // 0xCxkk
            reg.V[x] = randomByte() & kk;
            break;
        case 0xD: // 0xDxyz
            reg.V[0xF] = display.draw(loadSprite(reg.I, z), reg.V[x], reg.V[y]) ? 1 : 0;
            display.render();
            break;
        case 0xE:
            switch (kk) {
                case 0x9E:
                    printf("checking if key down %x\n", reg.V[x]);
                    if (keyDown[reg.V[x]]) {
                        reg.PC += 4;
                        PCModified = true;
                    }
                    break;
                case 0xA1:
                    printf("checking if key up %x\n", reg.V[x]);
                    if (!keyDown[reg.V[x]]) {
                        reg.PC += 4;
                        PCModified = true;
                    }
                    break;
                default:
                    printf(">>>>>>>>>>>>>>>>> ERROR!!!\n");
                    break;
            }
            break;
        case 0xF:
            switch (kk) {
                case 0x07:
                    reg.V[x] = reg.delayTimer.get();
                    break;
                case 0x0A:
                {
                    printf("waiting for any key press..\n");
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
                    }
                }
                    break;
                case 0x15:
                    reg.delayTimer.set(reg.V[x]);
                    break;
                case 0x18:
                    reg.soundTimer.set(reg.V[x]);
                    break;
                case 0x1E:
                    reg.I = reg.I + reg.V[x];
                    break;
                case 0x29:
                    reg.I = digitSpritePos[reg.V[x]];
                    break;
                case 0x33:
                    {
                        printf("BCD: %d\n", reg.V[x]);
                        uint8_t d100 = reg.V[x]/100;
                        uint8_t d10  = (reg.V[x]/10)%10;
                        uint8_t d    = reg.V[x]%10;
                        memory[reg.I]   = d100;
                        memory[reg.I+1] = d10;
                        memory[reg.I+2] = d;
                    }
                    break;
                case 0x55:
                    for (uint8_t r = 0; r <= x; ++r) {
                        memory[reg.I+r] = reg.V[r];
                    }
                    break;
                case 0x65:
                    for (uint8_t r = 0; r <= x; ++r) {
                        reg.V[r] = memory[reg.I+r];
                    }
                    break;
                default:
                    printf(">>>>>>>>>>>>>>>>> ERROR!!!\n");
                    break;
            }
            break;
    }

    if (!PCModified) {
        reg.PC += 2;
    }

    std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - startTime;
    printf("Took %.2lf ms\n", elapsed.count()*1000);
}

void Chip8Vm::pollEvents(bool& exit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        printf("Poll event: type %d, code %d\n", event.type, event.key.keysym.sym);
        switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                {
                    int keyCodes[] = {SDLK_1,SDLK_2,SDLK_3,SDLK_4,
                                      SDLK_q,SDLK_w,SDLK_e,SDLK_r,
                                      SDLK_a,SDLK_s,SDLK_d,SDLK_f,
                                      SDLK_z,SDLK_x,SDLK_c,SDLK_v};
                    for (int k = 0; k < 16; ++k) {
                        if (keyCodes[k] == event.key.keysym.sym) {
                            keyDown[k] = (event.type == SDL_KEYDOWN);
                        }
                    }
                }
                break;
        }
        if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
            display.render();
        }
        if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
            exit = true;
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
    bool exit = false;
    while (true) {
        pollEvents(exit);
        run();
        if (exit) {
            break;
        }
        SDL_Delay(10);
    }
}

uint8_t Chip8Vm::randomByte() {
    return rand() & 0xFF;
};
