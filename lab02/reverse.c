#include <fcntl.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <stdio.h> 

int main(int argc, char *argv[]){
    int wy = open(argv[1], O_RDONLY);
    int we = open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    char c;
    off_t file_size = lseek(wy, 0, SEEK_END);
    printf("%d", file_size);
    for (off_t i = 1; i <= file_size; ++i) {
        lseek(wy, -i, SEEK_END); 
        read(wy, &c, 1); 
        write(we, &c, 1);
    }
    close(we);
    close(wy);
    return 0;
}