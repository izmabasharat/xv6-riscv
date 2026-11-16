#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define R 1460
#define WARMUP 10

int
main(void)
{
    int p2c[2];
    int c2p[2];
    int single[2];

    char msg = 'A';
    char received;

    volatile int i;

    uint64 start, end;
    uint64 before, after;

    // --------------------------------
    // Measure pipe-call overhead
    // --------------------------------

    if(pipe(single) < 0) {
        printf("Pipe creation failed\n");
        exit(1);
    }

    start = clock();

    for(i = 0; i < R; i++) {
        write(single[1], &msg, 1);
        read(single[0], &received, 1);

        write(single[1], &msg, 1);
        read(single[0], &received, 1);
    }

    end = clock();

    uint64 pipe_time = end - start;

    close(single[0]);
    close(single[1]);

    // --------------------------------
    // Create two pipes
    // --------------------------------

    if(pipe(p2c) < 0 || pipe(c2p) < 0) {
        printf("Pipe creation failed\n");
        exit(1);
    }

    int pid = fork();

    if(pid < 0) {
        printf("Fork failed\n");
        exit(1);
    }

    if(pid == 0) {

        // Child process
        close(p2c[1]);
        close(c2p[0]);

        for(i = 0; i < WARMUP + R; i++) {
            if(read(p2c[0], &received, 1) != 1)
                exit(1);

            if(write(c2p[1], &msg, 1) != 1)
                exit(1);
        }

        close(p2c[0]);
        close(c2p[1]);

        exit(0);

    } else {

        // Parent process
        close(p2c[0]);
        close(c2p[1]);

        // Warm-up rounds
        for(i = 0; i < WARMUP; i++) {
            write(p2c[1], &msg, 1);
            read(c2p[0], &received, 1);
        }

        // Start measurement
        before = getswitches();
        start = clock();

        for(i = 0; i < R; i++) {
            if(write(p2c[1], &msg, 1) != 1)
                break;

            if(read(c2p[0], &received, 1) != 1)
                break;
        }

        end = clock();
        after = getswitches();

        wait(0);

        uint64 total_time = end - start;
        uint64 total_switches = after - before;

        // Convert to nanoseconds
        uint64 pipe_ns = pipe_time * 100;
        uint64 total_ns = total_time * 100;

        uint64 corrected_ns = 0;

        if(total_ns > pipe_ns)
            corrected_ns = total_ns - pipe_ns;

        uint64 ns_per_switch = 0;

        if(total_switches > 0)
            ns_per_switch = corrected_ns / total_switches;

        printf("\n--- CONTEXT SWITCH BENCHMARK ---\n");
        printf("Rounds: %d\n", R);
        printf("Total switches: %d\n",
               (int)total_switches);
        printf("Switches per round: %d\n",
               (int)(total_switches / R));
        printf("Total time: %d ns\n", (int)total_ns);
        printf("Pipe overhead: %d ns\n", (int)pipe_ns);
        printf("Corrected time: %d ns\n", (int)corrected_ns);
        printf("Time per context switch: %d ns\n",
               (int)ns_per_switch);

        close(p2c[1]);
        close(c2p[0]);

        exit(0);
    }
}