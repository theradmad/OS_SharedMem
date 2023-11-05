#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search_argument>\n", argv[0]);
        return 1;
    }

    int pipe1[2]; // Pipe between P1 and P2
    int pipe2[2]; // Pipe between P2 and P3
    int pid1, pid2, pid3;

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", argv[1], NULL};
    char *sort_args[] = {"sort", NULL};

    // Create pipe1
    if (pipe(pipe1) == -1) {
        perror("pipe1");
        return 1;
    }

    pid1 = fork();

    if (pid1 == 0) {
        // P1 (Child): Execute "cat scores"
        close(pipe1[0]); // Close read end of pipe1
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);

        execvp("cat", cat_args);
    }

    // Create pipe2
    if (pipe(pipe2) == -1) {
        perror("pipe2");
        return 1;
    }

    pid2 = fork();

    if (pid2 == 0) {
        // P2 (Child): Execute "grep [your argument]"
        close(pipe1[1]); // Close write end of pipe1
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);

        close(pipe2[0]); // Close read end of pipe2
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe2[1]);

        execvp("grep", grep_args);
    }

    pid3 = fork();

    if (pid3 == 0) {
        // P3 (Child): Execute "sort"
        close(pipe2[1]); // Close write end of pipe2
        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);

        execvp("sort", sort_args);
    }

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    // Wait for the child processes to finish
    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}

