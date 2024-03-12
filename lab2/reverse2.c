#include <fcntl.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <stdio.h> 

void swap(char c[], int i, int size) {
    char temp = c[i];
    c[i] = c[size - i - 1];
    c[size- i - 1] = temp;
}

void reverse_block(char c[], int size) {
    for (size_t j = 0; j < size / 2; ++j) {
        swap(c, j, size);
    }
}

int main(int argc, char *argv[]) {
    int BUFFER_SIZE = 2;
    int input_file = open(argv[1], O_RDONLY);
    int output_file = open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    char buffer[BUFFER_SIZE];
    off_t file_size = lseek(input_file, 0, SEEK_END);
    for (off_t i = 0; i < file_size; i += BUFFER_SIZE) {
        int size = i + BUFFER_SIZE < file_size ? BUFFER_SIZE : file_size - i;
        if (i + BUFFER_SIZE < file_size) {
            lseek(input_file, -i - BUFFER_SIZE, SEEK_END); 
        } else{
            lseek(input_file, 0, SEEK_SET); 
        }
        read(input_file, buffer, size);
        reverse_block(buffer, size);
        lseek(output_file, 0, SEEK_END);
        write(output_file, buffer, size);
    }
    close(output_file);
    close(input_file);
    return 0;
}