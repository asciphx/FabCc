
#include "uv.h"
#include "internal.h"

#include <stdint.h>
#include <time.h>

#undef NANOSEC
#define NANOSEC ((uint64_t) 1e9)
uint64_t uv__hrtime(uv_clocktype_t type) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return (((uint64_t) ts.tv_sec) * NANOSEC + ts.tv_nsec);}