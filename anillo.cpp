#include <stdio.h>
#include <cstring>
#include <cstdio>

#define BYTE unasigned char

//Functions

//void readFromFile();
void readFile(const char* filePath);

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


    const char* filePath = "nodo_1.txt"; // Replace "example.txt" with your file path
    readFile(filePath);


    // Return 0 to indicate successful program execution
    return 0;
    
}

//Fuctions

// void readFromFile(){ 

//     FILE * fp;
//     char * line = NULL;
//     size_t len = 0;
//     ssize_t read;

//     fp = fopen("nodo_1.txt", "r");

//     if(fp == NULL){

//         printf("Error open file\n");
//         return;

//     }

//     while((read = getline(&line, &len, fp)) != -1){

//         printf("%s",line);

//     }

//     fclose(fp);

// }


void readFile(const char* filePath) {
    FILE* inputFile = fopen(filePath, "r");
    if (inputFile == nullptr) {
        printf("Failed to open the file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), inputFile) != nullptr) {
        printf("%s", line);
    }

    fclose(inputFile);
}

