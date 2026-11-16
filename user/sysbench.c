#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define N 14600

int
main(void)
{
    volatile int i;
    uint64 start, end;
    uint64 loop_time;
    uint64 null_time;
    uint64 getpid_time;

    // Measure loop overhead
    start = clock();
    for(i = 0; i < N; i++) {
    }
    end = clock();

    loop_time = end - start;

    // Measure null() calls
    start = clock();
    for(i = 0; i < N; i++) {
        null();
    }
    end = clock();

    null_time = end - start;

    // Measure getpid() calls
    start = clock();
    for(i = 0; i < N; i++) {
        getpid();
    }
    end = clock();

    getpid_time = end - start;

    // Convert ticks to nanoseconds and remove loop overhead
    uint64 null_avg =
        ((null_time - loop_time) * 100) / N;

    uint64 getpid_avg =
        ((getpid_time - loop_time) * 100) / N;

    uint64 loop_ns = (loop_time * 100) / N;

    printf("\n--- SYSTEM CALL BENCHMARK ---\n");
    printf("Iterations: %d\n", N);
    printf("Loop overhead per iteration: %d ns\n",
           (int)loop_ns);
    printf("Average null() time: %d ns\n",
           (int)null_avg);
    printf("Average getpid() time: %d ns\n",
           (int)getpid_avg);

    exit(0);
}