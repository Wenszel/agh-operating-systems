#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
double f(double x) {
    return 4 / (x * x + 1);
}

struct timespec get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    return ts;
}

int main(int argc, char **argv) {
    if (argc != 3) {
      printf("args: <rec_width (double)> <number of processes");
      return 1;
    }

    struct timespec start = get_time();
    double rec_width = atof(argv[1]);
    int steps = round(1 / rec_width);
    int processes = atoi(argv[2]);
    int steps_per_process = round(steps/processes);
    printf("all %d steps %d steps per process %d\n", steps, steps_per_process, processes);
    if (steps_per_process == 0) {
      printf("Too many processes");
      return 1;
    }
    int rest = steps % steps_per_process;
    int fds[processes];
    for (double i = 0; i < processes; i++) {
      int fd[2];
      pipe(fd);
      fds[(int)i] = fd[0];
      if (fork() == 0) {
        double result = 0;
        double start = i * (steps_per_process * rec_width);
        if (i == processes - 1) {
          for (double j = 0; j < steps_per_process + rest; j ++) {
            double curr_pos = start + (j * rec_width);
            result +=  rec_width * f(curr_pos);
          }
        } else{
        for (double j = 0; j < steps_per_process; j++) {
          double curr_pos = start + (j * rec_width);
          result +=  rec_width * f(curr_pos);
        }
        }
        write(fd[1], &result, sizeof(double));
        close(fd[1]);
        return 0;
      } else {
        close(fd[1]);
      }
    }
    while (wait(NULL) > 0);
    double field = 0.0;
    for (int i = 0; i < processes; i++) {
        double r = 0.0;
        read(fds[i], &r, sizeof(double));
        // printf("Result: %f Field: %f \n", r, field);
        field += r;
    }

    struct timespec end = get_time();
    double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / pow(10, 9);


    printf("\n######################################\n");
    printf("Field: %f\n", field);
    printf("Precision is %f\n", rec_width);
    printf("Number of processes is %d\n", processes);
    printf ("Your calculations took %f seconds to run.\n", time );
    return 0;
}
