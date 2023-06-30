#include <stdio.h>
#include <cstring>
#include <cstdio>

#define BYTE unsigned char

//Functions
void leerArchivo(const char* rutaArchivo, char* variables[4]);
void converterMAC(const char* MAC, BYTE* destino);
bool compararMAC(BYTE MACdestino[6],const char *MAC);

char* varTxt[4];
bool emisor = false;
int nodo;


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

BYTE msg[300]; //Se guarda todo el mensaje recibido excepto los bytes de inicio y final 
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
        nodo = 1;
        printf("Nodo 1\n");
        const char* rutaArchivo = "nodo_1.txt"; 
        leerArchivo(rutaArchivo,varTxt);
    } else if (strcmp(argv[1], "2") == 0) {
        nodo = 2;
        printf("Nodo 2\n");
        const char* rutaArchivo = "nodo_2.txt"; 
        leerArchivo(rutaArchivo,varTxt);
    } else if (strcmp(argv[1], "3") == 0) {
        nodo = 3;
        printf("Nodo 3\n");
        const char* rutaArchivo = "nodo_3.txt"; 
        leerArchivo(rutaArchivo,varTxt);
    } else {
        // Print an error message if the node is not recognized
        printf("Error con el Nodo\n");
    }

    char *MAC = varTxt[0]; //Tambien se puede aplicar el ARREGLO DEL ANDY !!!!!!!!!!!!!!!!!!!!!
    int PinIn = *varTxt[1] - 48;
    int PinOut = *varTxt[2] - 48;
    int Clock = *varTxt[3] - 48;

    printf("M.A.C: %s\n",MAC);
    printf("PinIn: %d\n",PinIn);
    printf("PintOut: %d\n",PinOut);
    printf("Clock: %d\n",Clock);


    if(emisor){
        printf("Nodo: %d, Modo Emisor, M.A.C: %s\n\n",nodo,MAC);
    }else{
        printf("Nodo: %d, Modo Receptor, M.A.C: %s\n\n",nodo,MAC);
    }


    // BE:E9:B0:09:BE:E9

    BYTE BE = 190;
    BYTE E9 = 233;
    BYTE B0 = 176;

    //MAC
    msg[0] = BE;
    msg[1] = E9;
    msg[2] = B0;
    msg[3] = 9;
    msg[4] = BE;
    msg[5] = E9;

    //TTL
    msg[12] = 1;

    //Longitud
    msg[13] = 1; //LSB
    msg[14] = 0; //MSB

    


    int desfase = 1;

    //FCS
    msg[16+desfase] = 1;
    msg[17+desfase] = 0;
    msg[18+desfase] = 0;
    msg[19+desfase] = 0;

    // Destribucion Capa Ethernet 
    // Destino: [0,5]
    // Origen: [6,11]
    // TTL: [12]
    // Longitud: [13,14]
    // Data + Relleno: [15, 15 + Longitud]
    // FCS: [(15 + Longitud) + 1, (15 + Longitud) + 4]

    if(emisor == true){ 

       
    emisor = false;    
    }
    BYTE MACdestino[6] = {msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]};
    printf("Comparacion de mac:%d\n",compararMAC(MACdestino, MAC));

    if(compararMAC(MACdestino, MAC)){ //Compara si la MAC del destino es igual a la MAC del receptor
        
        int largoCapaEthernet = msg[13] + (msg[14] << 8);
        printf("Longitud: %d\n",largoCapaEthernet);

        
        int FCScapaEthernet = 0;
        int x = 0;
        for(int i = (16 + largoCapaEthernet); i <= (16 + largoCapaEthernet) + 4; i++){
        
            FCScapaEthernet =  FCScapaEthernet + (msg[i] << x*8);
            x++;
        }
        
        printf("FCS: %d\n",FCScapaEthernet);

    }else{
        msg[12] = msg[12] - 1; //Resta al TTL

        if(msg[12] == 0){ //No se renvia el mensaje 
        printf("Mensaje descartado por TTL\n");
        printf("TTL:%d\n",msg[12]);

        }else{ //Se renvia el mensaje
        printf("Reenviando...\n");
        printf("TTL:%d\n",msg[12]);

        }

    }







    // Return 0 to indicate successful program execution
    return 0;
    
}

//Fuctions
#define BYTE unsigned char
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


void converterMAC(const char* MAC, BYTE* destino){

    sscanf(MAC, "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX", &destino[0], &destino[1], &destino[2], &destino[3], &destino[4], &destino[5]);

}

bool compararMAC(BYTE MACdestino[6],const char *MAC){

    BYTE MACconvertido[6];
    converterMAC(MAC,MACconvertido);

    printf("Mac Convertido\n");
    for(int i = 0; i < 6 ; i++){
        printf("%d",MACconvertido[i]);
    }
    printf("\n");

    int contador = 0;

    for(int i = 0; i < 6; i++){
        if(MACconvertido[i] == MACdestino[i]){
            contador++;
        }
    }
    if(contador == 6){
        return true;
    }

    return false;
}
