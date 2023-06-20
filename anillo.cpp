#include <wiringPi.h>
#include <stdio.h>
#include <cstring>
#include <cstdio>

#define BYTE unsigned char

//Functions
void leerArchivo(const char* rutaArchivo, char* variables[4]);
void converterMAC(const char* MAC, BYTE* destino);
void createEthernetFrame(BYTE* destino, BYTE* origen, BYTE* data, BYTE* frame);
void leerBit(int pinIn,  BYTE *MAC);

char* varTxt[4];
bool emisor = false;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
volatile int j = 0; //Cuenta la cantidad de bits hasta formar un byte
BYTE c = 0; //Se forma y se guarda el byte actual recibido
BYTE d = 0; //Representa el valor previo de “c”
bool transmissionStarted = false; //Indica el estado de la transmisión 
bool finished = false; //Finaliza el programa 

BYTE C0 = 192; //Guarda el valor de decimal de C0 en C0
BYTE DB = 219; //Guarda el valor de decimal de DB en DB
BYTE DC = 220; //Guarda el valor de decimal de DC en DC
BYTE DD = 221; //Guarda el valor de decimal de DD en DD

BYTE msg[62]; //Se guarda todo el mensaje recibido excepto los bytes de inicio y final 
int index = 0; //Desface requerido para guardar el byte en caso de encontrar un DBDC y/o DBDD
int B = 0; //Cantidad de bytes recibidos
int FCS = 0; //Se guarda el valor del FCS calculada del receptor
int FCSEmisor = 0; //Se guarda el valor del FCS calculada del emisor
int contadorFSC = 0; //Cuenta la cantidad de FCS
int conteoDeDataEsperada = 0; //Cuenta si la data esperada es igual a la recibida
int contadorComparacionDeDatos = 0; //Cuenta la cantidad de veces en que la data es igual a la esperada en múltiples mensajes  
int contadorDeMsgRecibidos = 0; //Cuenta la cantidad de mensajes recibidos 
int contadorDeData = 0; //Cuenta la cantidad de datos (versos) recibidos 

float pMsgRecibidosCorrectamente;
float pMsgRecibidosConError;
float pMsgRecibidosSinErroresNoDetectados;
float pMsgRecibidosConErroresNoDetectados;


BYTE dataEsperada[] = "~ ~ Mama, just killed a man ~ ~"; //Es la data que espera el receptor del emisor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
        emisor = true;
    } else if (strcmp(argv[2], "0") == 0) {
        printf("Receptor\n");
        emisor = false;
    } else {
        // Print an error message if the mode is not recognized
        printf("Error con el Modo\n");
    }
 
    // Determine which node the program is running on
    if (strcmp(argv[1], "1") == 0) {
        printf("Nodo 1\n");
        const char* rutaArchivo = "nodo_1.txt"; 
        leerArchivo(rutaArchivo,varTxt);
    } else if (strcmp(argv[1], "2") == 0) {
        printf("Nodo 2\n");
        const char* rutaArchivo = "nodo_2.txt"; 
        leerArchivo(rutaArchivo,varTxt);
    } else if (strcmp(argv[1], "3") == 0) {
        printf("Nodo 3\n");
        const char* rutaArchivo = "nodo_3.txt"; 
        leerArchivo(rutaArchivo,varTxt);
    } else {
        // Print an error message if the node is not recognized
        printf("Error con el Nodo\n");
    }

    BYTE *MAC = varTxt[0]; //Tambien se puede aplicar el ARREGLO DEL ANDY !!!!!!!!!!!!!!!!!!!!!
    int PinIn = *varTxt[1] - 48;
    int PinOut = *varTxt[2] - 48;
    int Clock = *varTxt[3] - 48;

    printf("M.A.C: %s\n",MAC);
    printf("PinIn: %d\n",PinIn);
    printf("PintOut: %d\n",PinOut);
    printf("Clock: %d\n",Clock);

    BYTE origen[6];

    converterMAC(MAC, origen);

    printf("Origen: %02X:%02X:%02X:%02X:%02X:%02X\n", origen[0], origen[1], origen[2], origen[3], origen[4], origen[5]);


   

//              ____               _   _                           
//             / ___|   ___     __| | (_)   __ _    ___            
//   _____    | |      / _ \   / _` | | |  / _` |  / _ \     _____ 
//  |_____|   | |___  | (_) | | (_| | | | | (_| | | (_) |   |_____|
//             \____|  \___/   \__,_| |_|  \__, |  \___/           
//                                         |___/                     

    //Codigo para el Emisor
    if(emisor == true){ 


    emisor = false;    
    }
    //Codigo para el Receptor

    printf("Esperando...\n");









    // Return 0 to indicate successful program execution
    return 0;
    
}

//Fuctions

void leerArchivo(const char* rutaArchivo, char* variables[4]) {
    int numeroLinea = 1; // Start from line 1
    FILE* archivo = fopen(rutaArchivo, "r");
    if (archivo == nullptr) {
        printf("Error al abrir el archivo\n");
        return;
    }

    char Linea[256];
    int x = 0;
    while (fgets(Linea, sizeof(Linea), archivo) != nullptr) {
        // Check if the current line number matches the desired lines
        //printf("Linea(%d): %s",numeroLinea, Linea);
        if (numeroLinea == 2 || numeroLinea == 5 || numeroLinea == 8 || numeroLinea == 11) {
            // Make a copy of the line and assign it to the corresponding variable
            variables[x] = strdup(Linea);
            //printf("line#: %d, valor: %d\n",numeroLinea, x);
            x++;
        }
        numeroLinea++;
    }

    fclose(archivo);

}

void createEthernetFrame(BYTE* destino, BYTE* origen, BYTE* data, BYTE* frame){

    for(int i = 0; i < 6; i++){

        frame[i] = destino[i];
        frame[i + 6] = origen[i];

    }

}

void converterMAC(const char* MAC, BYTE* destino){

    sscanf(MAC, "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX", &destino[0], &destino[1], &destino[2], &destino[3], &destino[4], &destino[5]);

}

void leerBit(int pinIn, BYTE *MAC) {
  int largo; //Se inicializa el largo del mensaje 
  int cmd; //Se inicializa el comando del mensaje
  int comienzoData = 1; //Se inicializa el comienzo de la data
  bool bit = digitalRead(pinIn); //Se leen los bits del pin
  
  c = (c << 1) | bit; //Se guardan los bits en “c”

    if(transmissionStarted == false){
        if(c == C0){ //Si se detecta un C0 se inicia la transmisión y se resetean las variables
            printf("inicio\n");
            j = 0;
            B = 0;
            c = 0;
            d = 0;  
            FCS = 0;
            index = 0;
            FCSEmisor = 0;
            conteoDeDataEsperada = 0;
            transmissionStarted = true;
        }
    }else{
        printf ("%d", bit);
        j++; 

        if(j == 8){
            j = 0;   

            msg[B-index] = c; //Se guarda el byte de “c” en msg[] 

            if(c == C0){ //Se activa con el segundo C0 es decir con el byte de final

            //Chequeo de Mac con el destino


            }else{
                printf("  %d  %c\n",c,c);
            }


            if(d == DB & c == DC){ //Detecta DBDC
                printf("Se Recibio un: DBDC\n");
                
                msg[B-index] = 0; //Transforma el DB en 0 
                index++;
                msg[B-index] = C0; //Transforma el DC en C0
            }  
            if(d == DB & c == DD){ //Detecta DBDD
                printf("Se Recibio un: DBDD\n");

                msg[B-index] = 0; //Transforma el DB en 0
                index++;
                msg[B-index] = DB; //Transforma el DD en DB
            } 

            d = c;  
            B++;
        }
    }






}

