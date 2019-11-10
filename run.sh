#!/bin/bash
g++ -o test main.cpp chip8.cpp display.cpp chip8.h display.h font.h -lSDL2
if [ "$1" != "" ];
then
    ./test $1
fi
