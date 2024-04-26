#include <stdio.h>
#include "opencl.hpp"
#define R(...) string(" "#__VA_ARGS__" ")

struct Matrix
{
    float* line;
    float** rect;
    uint rows;
    uint cols;

    Matrix (uint rows, uint cols) {
        this->rows = rows;
        this->cols = cols;
        this->line = new float[rows * cols];
        this->rect = new float*[rows];
        for (uint i=0; i<rows; i++) {
            this->rect[i] = this->line + (i*cols);
            for (uint j=0; j<cols; j++) {
                this->rect[i][j] = 0.0f;
            }
        }
    }

    static bool can_mult (Matrix* a, Matrix* b) {
        return a->cols == b->rows;
    }

    static Matrix* empty_mult_result (Matrix* a, Matrix* b) {
        return new Matrix(a->rows, b->cols);
    }

    static Matrix* from_file (const char* filename) {
        FILE* source = fopen(filename, "r");
        if (source == NULL) {
            return NULL;
        }
        int rows = 0, cols = 0;
        fscanf(source, "%d %d", &rows, &cols);
        Matrix* result = new Matrix(rows, cols);
        if (rows == 0 || cols == 0) {
            fclose(source);
            return result;
        }
        for (int i=0; i<rows; i++) {
            if (feof(source)) break;
            for (int j=0; j<cols; j++) {
                if (feof(source)) break;
                float t = 0;
                fscanf(source, "%f", &t);
                result->rect[i][j] = t;
            }
        }
        fclose(source);
        return result;
    }
};

uint get_buffer_size_for_matrix_mult (Matrix* a, Matrix* b) {
    uint asize = a->rows * a->cols;
    uint bsize = b->rows * b->cols;
    uint csize = a->rows * b->cols;
    uint maxsize = max(asize, max(bsize, csize));
    uint result = 64;
    while (result < maxsize) result *= 64;
    return result;
}

Device* get_opencl () {
    Device* device = new Device(select_device_with_most_flops());
    printf("device: %s\n--------------------------\n", device->info.name.c_str());
    return device;
}

Device* get_opencl_for_matrix_mult () {
    Device* device = get_opencl();
    // stupid approach: let's encode matrices as 1d buffers
    // matrix_mult kernel would have signature
    // matrix_mult(global float* A, global float* B, global float* C, global uint* size)
    // size = { A.rows, A.cols, B.cols }
    // R is a stringification macro
    device->program(R(
        kernel void matrix_mult(global float* A, global float* B, global float* C, global uint* size) { 
            uint my_id = get_global_id(0);
            uint my_row = my_id / size[2];
            uint my_col = my_id % size[2];
            if (my_id < size[0] * size[2]) {
                uint K = size[1];
                uint a_cols = size[1];
                uint b_cols = size[2];
                float sum = 0.0f;
                for (uint k=0; k<K; k++) {
                    // expand A[my_row][k], expand B[k][my_row]
                    sum += A[my_row * a_cols + k] * B[k * b_cols + my_col];
                }
                C[my_id] = sum;
            }
        }
    ));

    return device;
}

void matrix_print (Matrix* m) {
    if (m == NULL) {
        printf("NULL\n--------------------------\n");
        return;
    }
    for (int i=0; i<m->rows; i+=1) {
        for (int j=0; j<m->cols; j+=1) {
            printf(" %8.2lf", m->rect[i][j]);
        }
        printf("\n");
    }
    printf("--------------------------\n");
}

int main (int argc, const char** argv) 
{
    if (argc < 3) {
        printf("usage: %s <file1> <file2> \n", argv[0]);
        return 1;
    }

    Matrix* Ma = Matrix::from_file(argv[1]);
    Matrix* Mb = Matrix::from_file(argv[2]);

    if (Ma) {
        printf("matrix A: \n");
        matrix_print(Ma);
    }
    else {
        printf("could not read matrix from %s \n", argv[1]);
        return 1;
    }
    if (Mb) {
        printf("matrix B: \n");
        matrix_print(Mb);
    }
    else {
        printf("could not read matrix from %s \n", argv[2]);
        return 1;
    }
    if (!Matrix::can_mult(Ma, Mb)) {
        printf("could not multiply matrices A * B \n");
        return 1;
    }

    Matrix* Mc = Matrix::empty_mult_result(Ma, Mb);

    Device* matrix_multiplier = get_opencl_for_matrix_mult(); 

    uint bufsize = get_buffer_size_for_matrix_mult(Ma, Mb);
    printf("needed buf size (rounded): %d \n--------------------------\n", bufsize);

    Memory<float> mema(matrix_multiplier[0], bufsize);
    Memory<float> memb(matrix_multiplier[0], bufsize);
    Memory<float> memc(matrix_multiplier[0], bufsize);
    Memory<uint> sizes(matrix_multiplier[0], bufsize);

    Kernel matrix_mult_kernel(matrix_multiplier[0], bufsize, "matrix_mult", mema, memb, memc, sizes);
    for (uint i=0; i<bufsize; i++) {
        mema[i] = 0.0f;
        memb[i] = 0.0f;
        if (i < (Ma->rows * Ma->cols)) mema[i] = Ma->line[i];
        if (i < (Mb->rows * Mb->cols)) memb[i] = Mb->line[i];
    } 
    sizes[0] = Ma->rows;
    sizes[1] = Ma->cols;
    sizes[2] = Mb->cols;

    mema.write_to_device();
    memb.write_to_device();
    memc.write_to_device();
    sizes.write_to_device();
    matrix_mult_kernel.run();
    memc.read_from_device();

    for (uint i=0; i<(Mc->rows * Mc->cols); i++) {
        Mc->line[i] = memc[i];
    }
    printf("matrix C = A * B: \n");
    matrix_print(Mc);

    return 0;
}