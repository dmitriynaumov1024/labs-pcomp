#include <stdio.h>
#include <string.h>
#include <math.h>
#include <CLWrapper/opencl.hpp>
#include "timeutils.cpp"
#include "sequence.cpp"

#define R(...) string(" "#__VA_ARGS__" ")

Device* get_opencl () {
    Device* device = new Device(select_device_with_most_flops());
    printf("device: %s\n--------------------------\n", device->info.name.c_str());
    return device;
}

Device* get_opencl_autocor () {
    Device* device = get_opencl();
    device->program(R(
        kernel void autocor (global float* A, global float* D, global int* sizes) {
            int asize = sizes[0], dsize = sizes[1];
            int t = get_global_id(0);
            if (t < dsize) {
                float sum = 0.0f;
                for (int i=0; i<asize; i++) {
                    sum += A[i] * A[(i+t)%asize];
                }
                D[t] += sum;
            }
        }
    ));
    return device;
}

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
void autocor (float* A, float* D, int asize, int dsize)
{
    Device* device = get_opencl_autocor();
    
    Memory<float> memA(device[0], asize);
    for (int i=0; i<asize; i++) { memA[i] = A[i]; }
    
    Memory<float> memD(device[0], dsize);

    Memory<int> sizes(device[0], 2);
    sizes[0] = asize; sizes[1] = dsize;

    Kernel autocor_kernel(device[0], dsize, "autocor", memA, memD, sizes);
    
    memA.write_to_device();
    sizes.write_to_device();

    autocor_kernel.run();
    memD.read_from_device();
    for (int i=0; i<dsize; i++) { D[i] = memD[i]; }
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
