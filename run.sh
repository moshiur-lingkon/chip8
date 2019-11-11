#!/bin/bash
g++ -o play src/*.cpp src/*.h -lSDL2
if [ "$1" != "" ];
then
    ./play $1
fi
