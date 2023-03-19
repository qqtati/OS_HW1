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
    int pipe_1[2];
    int pipe_2[2];
    char buffer[buf_size];
    char* string_yes = "Yes, string is palindrome";
    char* string_no = "No, string is not a palindrome";
    ssize_t read_bytes;
    ssize_t written_bytes;

    // Создаем неименнованные каналы
    if (pipe(pipe_1) < 0) {
        printf("pipe_1: Can\'t open pipe\n");
        exit(-1);
    }
    if (pipe(pipe_2) < 0) {
        printf("pipe_2: Can\'t open pipe\n");
        exit(-1);
    }

    // Создаем дочерние процессы
    fork_result = fork();
    if (fork_result < 0) {
        printf("fork: Can\'t fork child\n");
        exit(-1);
    } else if (fork_result > 0) {
        // Закрываем ненужные неименнованные каналы
        close(pipe_1[0]);
        close(pipe_2[0]);
        close(pipe_2[1]);
        printf("process_1: Reading from file %s\n", argv[1]);
        file_input_desc = open(argv[1], O_RDONLY);
        if (file_input_desc < 0) {
            printf("process_1: Can\'t open file\n");
            exit(-1);
        }
        read_bytes = read(file_input_desc, buffer, buf_size);
        if (read_bytes > 0) {
            written_bytes = write(pipe_1[1], buffer, read_bytes);
            printf("process_1: Send %ld bytes to pipe\n", written_bytes);
        }
        close(file_input_desc);
        close(pipe_1[1]);
        printf("process_1: Exit\n");
    } else {
        fork_result = fork();
        if (fork_result < 0) {
            printf("fork: Can\'t fork child\n");
            exit(-1);
        } else if (fork_result > 0) {
            // Закрываем ненужные неименнованные каналы
            close(pipe_1[1]);
            close(pipe_2[0]);
            read_bytes = read(pipe_1[0], buffer, buf_size);
            printf("process_2: Recieved %ld bytes from pipe\n", read_bytes);

            for (int i = 0; i < read_bytes / 2; ++i)
            {
                if (buffer[i] != buffer[read_bytes - 1 - i]) {
                    written_bytes = write(pipe_2[1], string_no, 30);
                    printf("process_2: String is not a palindrome\n");
                    close(pipe_2[1]);
                    close(pipe_1[0]);
                    printf("process_2: Exit\n");
                    return 0;
                }
            }
            written_bytes = write(pipe_2[1], string_yes, 25);
            printf("process_2: String is a palindrome\n");
            close(pipe_2[1]);
            close(pipe_1[0]);
            printf("process_2: Exit\n");
        } else {
            // Закрываем ненужные неименнованные каналы
            close(pipe_1[0]);
            close(pipe_1[1]);
            close(pipe_2[1]);
            file_output_desc = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
            if (file_output_desc < 0) {
                printf("process_3: Can\'t create file\n");
                exit(-1);
            }
            read_bytes = read(pipe_2[0], buffer, buf_size);
            printf("process_3: Recieved %ld byes from pipe\n", read_bytes);
            written_bytes = write(file_output_desc, buffer, read_bytes);
            printf("process_3: Writing to file %s %ld bytes\n", argv[2], written_bytes);
            close(file_output_desc);
            close(pipe_2[0]);
            printf("process_3: Exit\n");
        }
    }
    return 0;
}