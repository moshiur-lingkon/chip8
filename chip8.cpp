#include "chip8.h"

Chip8Vm::Chip8Vm() {
    reg.delayTimer.startVal = 0;
    reg.soundTimer.startVal = 0;
    reg.stackPointer = 0;
}

void Chip8Vm::run(Rom rom) {
    for (int i = 0; i < rom.size(); ++i) {
        uint16_t ins = rom.get(i);
        memory[0x200 + i] = ins>>8;
        memory[0x200 + i + 1] = ins&0xFF;
    }
    reg.PC = 0x200;
    while (true) {
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
                if (V[x] == kk) {
                    reg.PC += 2;
                }
                break;
            case 0x4: // 0x4xkk
                if (V[x] != kk) {
                    reg.PC += 2;
                }
                break;
            case 0x5: // 0x5xy0 TODO: check if z is 0
                if (V[x] == V[y]) {
                    reg.PC += 2;
                }
                break;
            case 0x6: // 0x6xkk
                V[x] = kk;
                reg.PC++;
                break;
            case 0x7: // 0x7xkk
                V[x] += kk;
                reg.PC++;
                break;
            case 0x8: // 0x
                switch (z) {
                    case 0:
                        V[x] = V[y];
                        break;
                    case 1:
                        V[x] |= V[y];
                        break;
                    case 2:
                        V[x] &= V[y];
                        break;
                    case 3:
                        V[x] ^= V[y];
                        break;
                    case 4:
                        uint16_t bigVx = V[x];
                        uint16_t bigVy = V[y];
                        uint16_t sum = bigVx + bigVy;
                        V[0xF] = sum > 255 ? 1 : 0;
                        V[x] = sum & 0xFFFF;
                        break;
                    case 5:
                        V[0xF] = V[x] > V[y] ? 1 : 0;
                        V[x] -= V[y]; // TODO: we are using unsgined??
                        break;
                    case 6:
                        V[0xF] = V[x]&1;
                        V[x] >>= 1;
                        break;
                    case 7:
                        V[0xF] = V[y] > V[x] ? 1 : 0;
                        V[x] = V[y] - v[x]; // TODO: we are using unsgined??
                        break;
                    case 0xE:
                        V[0xF] = V[x]>>7;
                        V[x] <<= 1;
                        break;
                }
                break;
        }
    }
}
