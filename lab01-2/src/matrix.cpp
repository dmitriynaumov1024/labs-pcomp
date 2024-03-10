#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MATRIX_CPP_INCLUDED
#define MATRIX_CPP_INCLUDED

template<typename T>
struct Matrix {
    int rows;
    int cols;
    T** data;

    Matrix (int rows, int cols) {
        this->rows = rows;
        this->cols = cols;
        this->data = new T*[rows];
        for (int i=0; i<this->rows; i+=1) {
            this->data[i] = new T[cols];
            for (int j=0; j<cols; j+=1) {
                memset((char*)this->data[i], 0, cols * sizeof(T));
            } 
        }
    }

    T* operator[] (int row) {
        return this->data[row];
    }

    ~Matrix () {
        for (int i=0; i<this->rows; i+=1) {
            delete[] this->data[i]; 
        }
        delete[] this->data;
    }
};

#endif
