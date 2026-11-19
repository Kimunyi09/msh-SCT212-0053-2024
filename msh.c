#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>

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

        
    }
    
/* Release the buffer allocated by getline */
    free(line);
    return 0;
}
