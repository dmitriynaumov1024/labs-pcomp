#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include "timeutils.cpp"
#include "sequence.cpp"

#define max_threads 8

// do necessary preparations
void init (int argc, char** argv)
{
    int num_threads = atoi(argv[2]);
    if (num_threads <= 0) num_threads = 1;
    if (num_threads > max_threads) num_threads = max_threads;
    omp_set_num_threads(num_threads);
    printf("OpenMP with %d threads \n", num_threads);
    return;
}

// params:
// A: signal values
// D: sums container
// asize: size of A
// dsize: size of D
// returns: nothing
// modifies: D
void autocor (float* A, float* D, int asize, int dsize)
{
    #pragma omp parallel for
    for (int t=0; t<dsize; t++) {
        float sum = 0.0f;
        for (int i=0; i<asize; i++) {
            sum += A[i]*A[(i+t)%asize];
        }
        D[t] = sum;
    }
}

void autocor_diff (float* D, int dsize)
{
    for (int i=1; i<dsize; i++) {
        D[i] = float(fabs(D[0] - D[i]));
    }
}

int main (int argc, char** argv) 
{
    if (argc < 3) {
        printf("Usage: %s <file> <threads>\n", argv[0]);
        return 1;
    }

    init(argc, argv);

    Sequence* A = Sequence::from_file(argv[1]);
    // Sequence* A = Sequence::mock_sinusoid(14000, 23, 10.0, 0.2);

    if (!A) {
        printf("Could not handle file %s\n", argv[1]);
        return 1;
    }

    Sequence* D = A->empty_autocor_result();

    timestamp start = hclock::now();

    autocor(A->data, D->data, A->size, D->size);
    autocor_diff(D->data, D->size);
    int T = D->index_min(1);
    float minDiff = D->min(1);

    timestamp end = hclock::now();

    printf("Signal size = %d entries\n", A->size);
    printf("Signal period = %d ticks\n", T);
    printf("Min diff = %.2f\n", minDiff);
    printf("Time spent = %.2lf ms\n", timediff(start, end));
    return 0;
}
