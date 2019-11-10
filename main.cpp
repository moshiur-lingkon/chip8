#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <endian.h>
#include <vector>
#include <SDL2/SDL.h>
#include "chip8.h"

std::vector<uint16_t> loadRomFromFile(FILE* f) {
    std::vector<uint16_t> ret;
    const size_t BUF_SIZE = 1024;
    uint16_t buffer[BUF_SIZE];
    while (true) {
        size_t readSize = fread(buffer, sizeof(uint16_t), BUF_SIZE, f);
        for (int i = 0; i < readSize; ++i) {
            ret.push_back(htobe16(buffer[i]));
        }
        if (readSize < BUF_SIZE) break;
    }
    return ret;
}

int main(int argc, char* argv[]) {
    FILE* romFile = fopen(argv[1], "rb");
    if (romFile == NULL) {
        printf("ERROR: couldn't open rom file: %s", argv[1]);
        return 1;
    }
    std::vector<uint16_t> rom = loadRomFromFile(romFile);
    fclose(romFile);

    //for (int i=0;i<rom.size();++i) {
    //    printf("%x\n", rom[i]);
    //}

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("failed to SDL_INIT_VIDEO, error %s", SDL_GetError());
        return 1;
    }

    srand(time(NULL));

    Chip8Vm chip8;

    chip8.emulate(rom);

    SDL_Quit();
    return 0;
}
