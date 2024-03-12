/*
* FILE: omp_hello.cpp
* compile: g++ -fopenmp omp_hello.cpp -o omp_hello
* run: ./omp_hello
*/
#include <omp.h>
#include <iostream>

int main() {
    int nthreads, tid;
    // Fork a team of threads giving their own copies of variables
    #pragma omp parallel private(nthreads, tid)
    {
        tid = omp_get_thread_num(); // obtain thread number
        std::cout << "Hello from the thread # " << tid << std::endl;
        // Only the master thread does this
        if (tid == 0)
        {
            nthreads = omp_get_num_threads();
            std::cout << "The number of threads is " << nthreads << std::endl;
        }
    }
    // All threads join the master thread and disband
    return 0;
}
