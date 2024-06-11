#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define FIFO_NAME "calc_fifo"

double f(double x) {
    return 4 / (x * x + 1);
}

int main() {
    double start, end;
    char buffer[256];

    int fd = open(FIFO_NAME, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    read(fd, buffer, sizeof(buffer));
    sscanf(buffer, "%lf %lf", &start, &end);

    close(fd);
    printf("range: %f %f\n", end, start);
    double output = 0.0;
    double step = 0.001;
    for (double x = start; x < end; x += step) {
        output += f(x) * step;
    }
    printf("output %f\n", output);
    fd = open(FIFO_NAME, O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    sprintf(buffer, "%lf", output);
    write(fd, buffer, strlen(buffer));
    
    close(fd);

    return 0;
}
