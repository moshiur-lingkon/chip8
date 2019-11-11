#!/bin/bash
g++ -o play main.cpp chip8.cpp display.cpp chip8.h display.h font.h -lSDL2
if [ "$1" != "" ];
then
    ./play $1
fi
