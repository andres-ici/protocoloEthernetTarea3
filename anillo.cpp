#include <stdio.h>
#include <cstring>

#define BYTE unasigned char

int main(int argc, char* argv[]) {
    // Print the number of command line arguments entered
    printf("You have entered %d arguments:\n", argc);

    // Print each command line argument entered
    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }

    // Determine whether the program is running in "emisor" or "receptor" mode
    if (strcmp(argv[2], "1") == 0) {
        printf("Emisor\n");
    } else if (strcmp(argv[2], "0") == 0) {
        printf("Receptor\n");
    } else {
        // Print an error message if the mode is not recognized
        printf("Error con el Modo\n");
    }
 
    // Determine which node the program is running on
    if (strcmp(argv[1], "1") == 0) {
        printf("Nodo 1\n");
    } else if (strcmp(argv[1], "2") == 0) {
        printf("Nodo 2\n");
    } else if (strcmp(argv[1], "3") == 0) {
        printf("Nodo 3\n");
    } else {
        // Print an error message if the node is not recognized
        printf("Error con el Nodo\n");
    }

    // Return 0 to indicate successful program execution
    return 0;
    
}



