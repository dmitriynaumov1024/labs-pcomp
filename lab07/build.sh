#!/bin/bash
mkdir -p bin
rm bin/program
g++ ./src/*.cpp -o bin/program -std=c++17 -pthread -I./include -L./lib -lOpenCL
