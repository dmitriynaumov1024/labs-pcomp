#include <stdio.h>
#include <string.h>
#include <math.h>
#include <experimental/simd>
#include "timeutils.cpp"
#include "sequence.cpp"

#define simd_size 4
namespace stx = std::experimental;
typedef stx::fixed_size_simd<float, simd_size> float_simd;

// do necessary preparations
void init (int argc, char** argv)
{
    return;
}

// params:
// A: signal values
// D: sums container
// asize: size of A
// dsize: size of D
// returns: nothing
// modifies: D
void autocor (const float* A, float* D, const int asize, const int dsize)
{
    int batches = asize / simd_size + 1;
    float_simd* a_original = new float_simd[batches];
    // init A original
    for (int i=0; i<batches; i++) {
        a_original[i] = float_simd();
        for (int k=0; k<simd_size; k++) {
            a_original[i][k] = A[(i*simd_size+k) % asize];
        }
    }

    for (int t=0; t<dsize; t++) {
        float sum = 0.0f;
        // init A shifted
        for (int i=0; i<batches; i++) {
            float_simd a_shifted;
            for (int k=0; k<simd_size; k++) {
                a_shifted[k] = A[(i*simd_size+k+t) % asize];
            }
            float_simd a_prod = a_original[i]*a_shifted;
            for (int k=0; k<simd_size; k++) {
                sum += a_prod[k];
            }
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
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    init(argc, argv);

    Sequence* A = Sequence::from_file(argv[1]);

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
