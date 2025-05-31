#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 200

void remove_quotes(char *arg) {
    size_t len = strlen(arg);
    if (len >= 2 && ((arg[0] == '"' && arg[len - 1] == '"') || (arg[0] == '\'' && arg[len - 1] == '\''))) {
        memmove(arg, arg + 1, len - 2);
        arg[len - 2] = '\0';
    }
}

int main() {
    char command[256];
    char *commands[MAX_COMMANDS];

    while (1) {
        printf("Shell> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL)
            break;

        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0)
            break;

        int command_count = 0;
        char *token = strtok(command, "|");
        while (token != NULL) {
            while (*token == ' ') token++;
            if (*token == '\0') {
                command_count = 0;
                break;
            }
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        int pipes[command_count - 1][2];
        for (int i = 0; i < command_count - 1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(1);
            }
        }

        for (int i = 0; i < command_count; i++) {
            pid_t pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(1);
            }

            if (pid == 0) {
                if (i != 0) {
                    if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                        perror("dup2");
                        exit(1);
                    }
                }
                if (i != command_count - 1) {
                    if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                        perror("dup2");
                        exit(1);
                    }
                }

                for (int j = 0; j < command_count - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                char *args[MAX_ARGS + 1];
                int arg_count = 0;
                char *arg = strtok(commands[i], " \t");
                while (arg != NULL) {
                    remove_quotes(arg);
                    args[arg_count++] = arg;
                    if (arg_count > MAX_ARGS) {
                        exit(1);
                    }
                    arg = strtok(NULL, " \t");
                }
                args[arg_count] = NULL;

                if (arg_count == 0) {
                    exit(1);
                }

                execvp(args[0], args);
                exit(1);
            }
        }

        for (int i = 0; i < command_count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        for (int i = 0; i < command_count; i++) {
            int status;
            wait(&status);
        }
    }
    return 0;
}