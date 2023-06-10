#include <stdio.h>
#include <cstring>
#include <cstdio>

#define BYTE unasigned char

//Functions
void readFile(const char* filePath, char* variables[4]);



char* txt[4];

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
        const char* filePath = "nodo_1.txt"; 
        readFile(filePath,txt);
    } else if (strcmp(argv[1], "2") == 0) {
        printf("Nodo 2\n");
        const char* filePath = "nodo_2.txt"; 
        readFile(filePath,txt);
    } else if (strcmp(argv[1], "3") == 0) {
        printf("Nodo 3\n");
        const char* filePath = "nodo_3.txt"; 
        readFile(filePath,txt);
    } else {
        // Print an error message if the node is not recognized
        printf("Error con el Nodo\n");
    }

    printf("M.A.C: %s\n",txt[0]);
    printf("PinIn: %s\n",txt[1]);
    printf("PintOut: %s\n",txt[2]);
    printf("Clock: %s\n",txt[3]);
 


    // Return 0 to indicate successful program execution
    return 0;
    
}

//Fuctions

void readFile(const char* filePath, char* variables[4]) {
    int lineNumber = 1; // Start from line 1
    FILE* inputFile = fopen(filePath, "r");
    if (inputFile == nullptr) {
        printf("Failed to open the file.\n");
        return;
    }

    char line[256];
    int x = 0;
    while (fgets(line, sizeof(line), inputFile) != nullptr) {
        // Check if the current line number matches the desired lines
        printf("line(%d): %s",lineNumber, line);
        if (lineNumber == 2 || lineNumber == 5 || lineNumber == 8 || lineNumber == 11) {
            // Make a copy of the line and assign it to the corresponding variable
            variables[x] = strdup(line);
            //printf("line#: %d, valor: %d\n",lineNumber, x);
            x++;
        }
        lineNumber++;
    }

    fclose(inputFile);

}

