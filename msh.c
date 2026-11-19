#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_ARGS 64

int main(void) {
    char *line = NULL;
    size_t cap = 0;
    ssize_t nread;

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
        /* Print parsed tokens to verify tokenizer output */
        for (int i = 0; argv[i] != NULL; i++) {
            printf("argv[%d]: \"%s\"\n", i, argv[i]);
        }

        
    }
    
/* Release the buffer allocated by getline */
    free(line);
    return 0;
}
