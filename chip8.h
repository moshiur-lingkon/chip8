#ifndef __CHIP8_H__
#define __CHIP8_H__

struct Timer {
    uint8_t startVal;
    uint32_t setTime;

    uint8_t val(uint32_t now) {
        uint32_t elapsed = (now - setTime)/1000;
        if (elapsed < startVal) {
            return startVal - elapsed;
        }
        return 0;
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
    Registers reg;
    uint16_t stack[32];
    uint8_t memory[0x1000];
public:
    void run(Rom rom);
};

#endif
