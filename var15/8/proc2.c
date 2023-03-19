#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>


const int buf_size = 5000;

int main(int argc, char ** argv) {
    int file_input_desc;
    int file_output_desc;
    int pipe_1;
    int pipe_2;
    char buffer[buf_size];
    char* string_yes = "Yes, string is palindrome";
    char* string_no = "No, string is not a palindrome";
    char* pipe_1_name = "pipe_1.fifo";
    char* pipe_2_name = "pipe_2.fifo";
    ssize_t read_bytes;
    ssize_t written_bytes;

    // Создаем именнованные каналы
    mknod(pipe_1_name, S_IFIFO | 0666, 0);
    mknod(pipe_2_name, S_IFIFO | 0666, 0);


    if((pipe_1 = open(pipe_1_name, O_RDONLY)) < 0){
        printf("process_2: Can\'t open FIFO for reading\n");
        exit(-1);
    }
    if((pipe_2 = open(pipe_2_name, O_WRONLY)) < 0){
        printf("process_2: Can\'t open FIFO for writting\n");
        exit(-1);
    }
    read_bytes = read(pipe_1, buffer, buf_size);
    printf("process_2: Recieved %ld bytes from pipe\n", read_bytes);

    for (int i = 0; i < read_bytes / 2; ++i)
    {
        if (buffer[i] != buffer[read_bytes - 1 - i]) {
            written_bytes = write(pipe_2, string_no, 30);
            printf("process_2: String is not a palindrome\n");
            close(pipe_1);
            close(pipe_2);
            printf("process_2: Exit\n");
            return 0;
        }
    }
    written_bytes = write(pipe_2, string_yes, 25);
    printf("process_2: String is a palindrome\n");
    close(pipe_2);
    close(pipe_1);
    printf("process_2: Exit\n");  
    return 0;
}