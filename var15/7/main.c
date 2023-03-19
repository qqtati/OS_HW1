#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>


const int buf_size = 5000;

int main(int argc, char ** argv) {
    if (argc < 3) {
        printf("Usage: main <input> <output>\n");
        exit(0);
    }
    int file_input_desc;
    int file_output_desc;
    int fork_result;
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

    // Создаем дочерние процессы
    fork_result = fork();
    if (fork_result < 0) {
        printf("fork: Can\'t fork child\n");
        exit(-1);
    } else if (fork_result > 0) {
        // Открываем канал на запись
        if((pipe_1 = open(pipe_1_name, O_WRONLY)) < 0){
            printf("process_1: Can\'t open FIFO for writting\n");
            exit(-1);
        }
        printf("process_1: Reading from file %s\n", argv[1]);
        file_input_desc = open(argv[1], O_RDONLY);
        if (file_input_desc < 0) {
            printf("process_1: Can\'t open file\n");
            exit(-1);
        }
        read_bytes = read(file_input_desc, buffer, buf_size);
        if (read_bytes > 0) {
            written_bytes = write(pipe_1, buffer, read_bytes);
            printf("process_1: Send %ld bytes to pipe\n", written_bytes);
        }
        close(file_input_desc);
        close(pipe_1);

        // Открываем канал на чтение
        if((pipe_2 = open(pipe_2_name, O_RDONLY)) < 0){
            printf("process_1: Can\'t open FIFO for reading\n");
            exit(-1);
        }
        file_output_desc = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
        if (file_output_desc < 0) {
            printf("process_1: Can\'t create file\n");
            exit(-1);
        }
        read_bytes = read(pipe_2, buffer, buf_size);
        printf("process_1: Recieved %ld byes from pipe\n", read_bytes);
        written_bytes = write(file_output_desc, buffer, read_bytes);
        printf("process_1: Writing to file %s %ld bytes\n", argv[2], written_bytes);
        close(file_output_desc);
        close(pipe_2);
        printf("process_1: Exit\n");
    } else {
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
    }
    return 0;
}