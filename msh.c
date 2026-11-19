#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_ARGS 64
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    char *line = NULL;
    size_t cap = 0;
    ssize_t nread;
    int last_status = 0;

    while (1) {
    /* Display the shell prompt and flush stdout immediately */
        printf("msh> ");
        fflush(stdout); 
        
/* Read a line of arbitrary length from standard input */
        nread = getline(&line, &cap, stdin);
        if (nread == -1) {
                  
          /* Print a tidy newline and exit the loop on EOF */
            printf("\n");
            break;
        }
        /* Tokenize the input line into argv */
        char *argv[MAX_ARGS];
        int argc = 0;

        char *token = strtok(line, " \t\n");
        while (token != NULL && argc < MAX_ARGS - 1) {
            argv[argc++] = token;
            token = strtok(NULL, " \t\n");
        }
        /* Terminate argv with NULL as required by execvp */
        argv[argc] = NULL;

        /* Skip blank or whitespace-only lines */
        if (argv[0] == NULL) {
            continue;
        }
        /* Handle built-in commands before forking */
        if (strcmp(argv[0], "exit") == 0) {
            break;
        } else if (strcmp(argv[0], "help") == 0) {
            printf("msh built-in commands:\n");
            printf("  cd [dir]  Change the current directory (defaults to HOME)\n");
            printf("  help      Display this help message\n");
            printf("  exit      Exit the shell\n");
            last_status = 0;
            continue;
        } else if (strcmp(argv[0], "cd") == 0) {
            const char *target = argv[1];
            if (target == NULL) {
                target = getenv("HOME");
                if (target == NULL) {
                    fprintf(stderr, "cd: HOME not set\n");
                    last_status = 1;
                    continue;
                }
            }
            if (chdir(target) != 0) {
                perror("cd");
                last_status = 1;
            } else {
                last_status = 0;
            }
            continue;
        }
        //* Fork a child process to execute the external command */
        pid_t pid = fork();
        if (pid == -1) {
            /* Fork failed; report error and return to prompt */
            perror("fork");
            continue;
        } else if (pid == 0) {
            /* Child process replaces its image with the requested command */
            execvp(argv[0], argv);
            perror(argv[0]);
            _exit(127);
        } else {
            /* Parent process waits for the child to finish and records status */
            if (waitpid(pid, &last_status, 0) == -1) {
                perror("waitpid");
            }
        }

        
    }
    
/* Release the buffer allocated by getline */
    free(line);
    return 0;
}
