#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "matrix.cpp"

template<typename T>
struct matrix_mult_params {
    // sources
    Matrix<T>* a;
    Matrix<T>* b;
    // target
    Matrix<T>* c;
    // target cell coords
    int row;
    int col;
    // thread params in the same struct, why not?
    int offset;
    int step;
};

// one row/col of matrix mult
void matrix_mult (matrix_mult_params<double>* params) {
    Matrix<double>* a = params->a;
    Matrix<double>* b = params->b;
    Matrix<double>* c = params->c;
    int row = params->row;
    int col = params->col;
    double sum = 0;
    for (int k=0; k<a->cols; k++) {
        sum += a->data[row][k] * b->data[k][col];
    }
    c->data[row][col] = sum;
    return;
}

// one thread of parallel matrix multiplication
void* parallel_mult_thread (void* arg) {
    matrix_mult_params<double>* params = (matrix_mult_params<double>*) arg;
    Matrix<double>* a = params->a;
    Matrix<double>* b = params->b;
    Matrix<double>* c = params->c;
    int cols = c->cols, 
        rows = c->rows, 
        N = cols * rows,
        offset = params->offset, 
        step = params->step;
    for (int i=offset; i<N; i+=step) {
        params->row = i / cols;
        params->col = i % cols;
        matrix_mult(params);
    }
    return NULL;
}

// entry point for parallel matrix multiplication
Matrix<double>* parallel_mult (Matrix<double>* a, Matrix<double>* b, int num_threads) {
    if (a->cols != b->rows) {
        return NULL;
    }
    if (num_threads <= 0) {
        return NULL;
    }
    Matrix<double>* result = new Matrix<double>(a->rows, b->cols);
    matrix_mult_params<double>* params = new matrix_mult_params<double>[num_threads];
    pthread_t* threads = new pthread_t[num_threads];
    for (int i=0; i<num_threads; i++) {
        matrix_mult_params<double>* p = params + i;
        p->a = a;
        p->b = b;
        p->c = result;
        p->offset = i;
        p->step = num_threads;
        // create and start threads
        pthread_create(threads + i, NULL, parallel_mult_thread, p);
        printf("[debug]: parallel_mult: created and started #%d \n", i);
    }
    for (int i=0; i<num_threads; i++) {
        // wait for each thread
        pthread_join(threads[i], NULL);
        printf("[debug]: parallel_mult: successfully awaited #%d \n", i);
    }
    delete[] params;
    delete[] threads;
    return result;
}

// entry point for sequential matrix multiplication
Matrix<double>* sequential_mult (Matrix<double>* a, Matrix<double>* b) {
    if (a->cols != b->rows) {
        return NULL;
    }
    Matrix<double>* result = new Matrix<double>(a->rows, b->cols);
    for (int i=0; i<a->rows; i++) {
        for (int j=0; j<b->cols; j++) {
            matrix_mult_params<double> p;
            p.a = a;
            p.b = b;
            p.c = result;
            p.row = i;
            p.col = j;
            matrix_mult(&p);
        }
    }
    return result;
}

// params: 1: char* filename as void*
// returns: Matrix<double>* as void*
void* read_thread (void* arg) {
    char* filename = (char*)arg;
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
    Matrix<double>** result = new Matrix<double>*[count];
    for (int i=0; i<count; i+=num_threads) {
        pthread_t* threads = new pthread_t[num_threads];
        for (int j=0; j<num_threads && j+i<count; j++) {
            pthread_create(threads + j, NULL, read_thread, filenames[i+j]);
            printf("[debug]: read: created and started #%d \n", j);
        }
        for (int j=0; j<num_threads && j+i<count; j++) {
            pthread_join(threads[j], (void**)(result+i+j));
            printf("[debug]: read: successfully awaited #%d \n", j);
        }
        delete[] threads;
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
