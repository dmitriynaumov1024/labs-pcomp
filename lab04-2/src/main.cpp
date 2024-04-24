#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "matrix.cpp"

// one thread of parallel matrix multiplication
void parallel_mult_thread (Matrix<double>* a, Matrix<double>* b, Matrix<double>* c, int offset, int step) {
    int cols = c->cols, 
        rows = c->rows,
        N = rows * cols;
    for (int i=offset; i<N; i+=step) {
        int row = i / cols;
        int col = i % cols;
        double sum = 0;
        for (int k=0; k<a->cols; k++) {
            sum += a->data[row][k] * b->data[k][col];
        }
        c->data[row][col] = sum;
    }
}

// entry point for parallel matrix multiplication
Matrix<double>* parallel_mult (Matrix<double>* a, Matrix<double>* b, int num_threads) {
    if (a->cols != b->rows) {
        return NULL;
    }
    if (num_threads <= 0) {
        return NULL;
    }
    omp_set_num_threads(num_threads);
    Matrix<double>* result = new Matrix<double>(a->rows, b->cols);
    #pragma omp parallel 
    {
        int offset = omp_get_thread_num();
        int step = omp_get_num_threads();
        parallel_mult_thread(a, b, result, offset, step);
    }
    return result;
}

// params: 1: char* filename as void*
// returns: Matrix<double>* as void*
Matrix<double>* read_thread (char* filename) {
    FILE* source = fopen(filename, "r");
    if (source == NULL) {
        return NULL;
    }
    int rows = 0, cols = 0;
    fscanf(source, "%d %d", &rows, &cols);
    Matrix<double>* result = new Matrix<double>(rows, cols);
    if (rows == 0 || cols == 0) {
        fclose(source);
        return result;
    }
    for (int i=0; i<rows; i++) {
        if (feof(source)) break;
        for (int j=0; j<cols; j++) {
            if (feof(source)) break;
            double t = 0;
            fscanf(source, "%lf", &t);
            result->data[i][j] = t;
        }
    }
    fclose(source);
    return result;
}

// entry point for parallel matrix reading from files, support any number of files
Matrix<double>** read (char** filenames, int count, int num_threads) {
    omp_set_num_threads(num_threads);
    Matrix<double>** result = new Matrix<double>*[count];
    for (int i=0; i<count; i+=num_threads) {
        #pragma omp parallel 
        {
            int j = omp_get_thread_num();
            if (i + j < count) {
                result[i + j] = read_thread(filenames[i + j]);
            }
        }
    }
    return result;
}

void matrix_print (Matrix<double>* m) {
    if (m == NULL) {
        printf("NULL\n\n");
        return;
    }
    for (int i=0; i<m->rows; i+=1) {
        for (int j=0; j<m->cols; j+=1) {
            printf(" %8.2lf", m->data[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}


#define max_threads 16

// main entry point
int main (int argc, char** argv) {
    if (argc < 4) {
        printf("usage: %s <filename 1> <filename 2> <threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[3]);
    if (num_threads <= 0) num_threads = 1;
    if (num_threads > max_threads) num_threads = max_threads;

    Matrix<double>** mats = read(argv + 1, 2, 2);
    Matrix<double>* m = mats[0];
    Matrix<double>* n = mats[1];

    printf("Matrix M:\n");
    matrix_print(m);

    printf("Matrix N:\n");
    matrix_print(n);

    Matrix<double>* p = parallel_mult(m, n, 2);
    printf("Matrix P = M x N:\n");
    matrix_print(p);

    return 0;
}
