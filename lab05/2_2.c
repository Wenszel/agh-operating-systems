#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FIFO_NAME "calc_fifo"

int main() {
    double start, end;

    printf("Podaj przedział");
    scanf("%lf %lf", &start, &end);

    if (mkfifo(FIFO_NAME, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    int fd_write = open(FIFO_NAME, O_WRONLY);
    sprintf(buffer, "%lf %lf", start, end);
    write(fd_write, buffer , sizeof(buffer));

    close(fd_write);

    int fd_read = open(FIFO_NAME, O_RDONLY);
    if (fd_read == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    double wynik;
    read(fd_read, buffer, sizeof(buffer));
    sscanf(buffer, "%lf", &wynik);
    close(fd_read);

    printf("Wynik całki: %lf\n", wynik);

    unlink(FIFO_NAME);

    return 0;
}
