#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search_argument>\n", argv[0]);
        return 1;
    }

    int pipefd[2];
    pid_t pid1, pid2, pid3;

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", argv[1], NULL};
    char *sort_args[] = {"sort", NULL};

    pipe(pipefd);

    pid1 = fork();

    if (pid1 == 0) {
        // Child process 1 (P1) executes cat scores
        dup2(pipefd[1], 1); // Redirect stdout to the write end of the pipe
        close(pipefd[0]); // Close the read end of the pipe
        execvp("cat", cat_args);
    }

    close(pipefd[1]); // Close the write end of the pipe

    pid2 = fork();

    if (pid2 == 0) {
        // Child process 2 (P2) executes grep <search_argument>
        dup2(pipefd[0], 0); // Redirect stdin to the read end of the pipe
        close(pipefd[1]); // Close the write end of the pipe
        execvp("grep", grep_args);
    }

    pid3 = fork();

    if (pid3 == 0) {
        // Child process 3 (P3) executes sort
        dup2(pipefd[0], 0); // Redirect stdin to the read end of the pipe
        close(pipefd[1]); // Close the write end of the pipe
        execvp("sort", sort_args);
    }

    close(pipefd[0]); // Close the read end of the pipe

    // Wait for all child processes to complete
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    return 0;
}
