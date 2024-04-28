#!/bin/bash

rm -r bin
mkdir bin

echo "** 01-sequential: start building...**"
g++ ./src/01-sequential.cpp -o ./bin/01-sequential
echo "** 01-sequential: done.**"

echo "** 02-openmp: start building...**"
g++ ./src/02-openmp.cpp -o ./bin/02-openmp -fopenmp
echo "** 02-openmp: done.**"

echo "** 03-simd: start building...**"
g++ ./src/03-simd.cpp -o ./bin/03-simd
echo "** 03-simd: done.**"

echo "** 04-opencl: start building...**"
g++ ./src/kernel.cpp ./src/04-opencl.cpp -o ./bin/04-opencl -std=c++17 -pthread -I./include -L./lib -lOpenCL
echo "** 04-opencl: done.**"
