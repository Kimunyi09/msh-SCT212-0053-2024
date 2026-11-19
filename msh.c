#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_ARGS 64
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(void) {
    char *line = NULL;
    size_t cap = 0;
    ssize_t nread;
    int last_status = 0;

    while (1) {
    /* Reap any finished background child processes before printing prompt */
        int st;
        while (waitpid(-1, &st, WNOHANG) > 0) {
        }
        
           /* Display prompt with the exit status of the previous foreground command */
        printf("msh[%d]> ", last_status);
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
        
        /* Check for trailing "&" to run command in the background */
        int background = 0;
        if (argc > 0 && strcmp(argv[argc - 1], "&") == 0) {
            background = 1;
            argv[argc - 1] = NULL;
            argc--;
            if (argv[0] == NULL) {
                continue;
            }
        }
        /* Scan argv for output redirection token ">" */
        char *outfile = NULL;
        int redirect_error = 0;
        for (int i = 0; argv[i] != NULL; i++) {
            if (strcmp(argv[i], ">") == 0) {
                if (argv[i + 1] == NULL) {
                    fprintf(stderr, "msh: syntax error near unexpected token `newline'\n");
                    last_status = 1;
                    redirect_error = 1;
                    break;
                }
                outfile = argv[i + 1];
                argv[i] = NULL;
                break;
            }
        }
        if (redirect_error) {
            continue;
        }
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
            /* Redirect stdout in child process only if ">" was specified */
            if (outfile != NULL) {
                int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror(outfile);
                    _exit(1);
                }
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("dup2");
                    close(fd);
                    _exit(1);
                }
                close(fd);
            }

            /* Child process replaces its image with the requested command */
            execvp(argv[0], argv);
            perror(argv[0]);
            _exit(127);
        } else {
            /* Parent process either prints background PID or waits for foreground child */
            if (background) {
                printf("[started pid %d]\n", pid);
            } else {
                int status;
                if (waitpid(pid, &status, 0) == -1) {
                    perror("waitpid");
                } else if (WIFEXITED(status)) {
                    last_status = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    last_status = 128 + WTERMSIG(status);
                }
            }
        }
        
    }
    
/* Release the buffer allocated by getline */
    free(line);
    return 0;
}
