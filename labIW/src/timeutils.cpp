#ifndef TIMEUTILS_CPP_INCLUDED
#define TIMEUTILS_CPP_INCLUDED

#include <chrono>

#define hclock_ticks_per_sec 1000000000

using namespace std::chrono;
typedef high_resolution_clock hclock;
typedef high_resolution_clock::time_point timestamp;

// difference between earlier and later timestamp
// in milliseconds
double timediff (timestamp start, timestamp end) {
    return double((end-start).count()) / hclock_ticks_per_sec * 1000;
}

#endif // TIMEUTILS_CPP_INCLUDED
