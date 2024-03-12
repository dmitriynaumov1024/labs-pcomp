#include <omp.h>
#include <iostream>

using namespace std;

int main() 
{
    int nthreads, tid;
    omp_set_num_threads(8);
    // Fork a team of threads giving their own copies of variables
    #pragma omp parallel private(nthreads, tid)
    {
        tid = omp_get_thread_num(); // obtain thread number
        #pragma omp critical 
        {
            cout << "Hello from the thread # " << tid << endl;
        }
        // Only the master thread does this
        if (tid == 0) 
        {
            nthreads = omp_get_num_threads();
            cout << "The number of threads is " << nthreads << endl;
        }
    }
    // All threads join the master thread and disband
    return 0;
}
