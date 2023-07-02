#include <wiringPi.h>
#include <stdio.h>
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <math.h>

#define BYTE unsigned char

//Funciones
void leerArchivo(const char* rutaArchivo, char* variables[4]);
void converterMAC(const char* MAC, BYTE* destino);

//Funciones para la generacion del Frame

int contNoNullChar(BYTE* array);
void createOurProtocolFrame(int cmd, int longitud, BYTE* data, BYTE* frame);
void createEthernetFrame(BYTE* destino, BYTE* origen, int longitud, BYTE* data, BYTE* frame);
int contFCSByte(BYTE byte);
int contLengthSLIP(BYTE* frame, int lenghtFrame);
void generateFrameSLIP(BYTE* frame, BYTE* frameSLIP, int lenghtFrame);

//Funciones de envio de datos

void generateFrameSLIP(BYTE* frame, BYTE* frameSLIP, int lenghtFrame);
void startTransmission();
void sendFrame(void);

//Variables

bool emisor;
int nodo;
char* varTxt[4];
int largoSlipProtocol;
int PinOut;

BYTE slipProtocol[100];
volatile int nbits = 0; //Cuenta la cantidad de bits recibidos
volatile int nbytes = 0; //Cuenta el número de Byte enviados

//Variables de envio de datos

bool transmissionStarted = false; //Indica el estado de la transmisión 

int main(int argc, char* argv[]){

    if(wiringPiSetup() == -1){ //Verificar correcto funcionamiento de Raspberry Pi

        printf("Error");

    }

    printf("You have entered %d arguments:\n", argc);

    for(int i = 0; i < argc; i++){

        printf("%s\n", argv[i]);

    }

    // Determine whether the program is running in "emisor" or "receptor" mode
    if(strcmp(argv[2], "1") == 0){

        printf("Emisor\n");
        emisor = true;

    }else if(strcmp(argv[2], "0") == 0){

        printf("Receptor\n");
        emisor = false;

    }else{

        // Print an error message if the mode is not recognized
        printf("Error con el Modo\n");

    }

    // Determine which node the program is running on
    if(strcmp(argv[1], "1") == 0){

        printf("Nodo 1\n");
        nodo = 1;
        const char* rutaArchivo = "nodo_1.txt"; 
        leerArchivo(rutaArchivo,varTxt);

    }else if(strcmp(argv[1], "2") == 0){

        printf("Nodo 2\n");
        nodo = 2;
        const char* rutaArchivo = "nodo_2.txt"; 
        leerArchivo(rutaArchivo,varTxt);

    }else if (strcmp(argv[1], "3") == 0){

        printf("Nodo 3\n");
        nodo = 3;
        const char* rutaArchivo = "nodo_3.txt"; 
        leerArchivo(rutaArchivo,varTxt);

    }else{

        // Print an error message if the node is not recognized
        printf("Error con el Nodo\n");

    }

    char *MAC = varTxt[0]; 
    int PinIn = atoi(varTxt[1]);
    PinOut = atoi(varTxt[2]);
    int Clock = atoi(varTxt[3]);

    printf("M.A.C: %s",MAC);
    printf("PinIn: %d\n",PinIn);
    printf("PintOut: %d\n",PinOut);
    printf("Clock: %d\n",Clock);

    pinMode(PinOut, OUTPUT);

    //CONFIGURA INTERRUPCION PIN CLOCK (PUENTEADO A PIN PWM)
    if(wiringPiISR(Clock, INT_EDGE_FALLING, &sendFrame) < 0){

        printf("Unable to start interrupt function\n");

    }

    if(emisor){

        printf("Nodo: %d, modo Emisor, M.A.C: %s",nodo, MAC);

    }else{

        printf("Nodo: %d, modo Receptor, M.A.C: %s",nodo, MAC);

    }

    BYTE origen[6];

    converterMAC(MAC, origen);

    printf("Origen: %02X:%02X:%02X:%02X:%02X:%02X\n", origen[0], origen[1], origen[2], origen[3], origen[4], origen[5]);

    if(emisor == true){ // Codigo para el Emisor 
    
        //Menu de opciones
        printf("\n\nPrograma modo emisor\n");
        char* nodo1[4];
        char* nodo2[4];
        char* nodo3[4];
        leerArchivo("nodo_1.txt", nodo1);
        leerArchivo("nodo_2.txt", nodo2);
        leerArchivo("nodo_3.txt", nodo3);
        printf("M.A.C nodo 1: %s", nodo1[0]);
        printf("M.A.C nodo 2: %s", nodo2[0]);
        printf("M.A.C nodo 3: %s", nodo3[0]);


        int nodoDestinoEmisor;
        printf("Ingresar numero del nodo de destino\n");
        scanf("%i", &nodoDestinoEmisor);

        BYTE destino[6];

        if(nodoDestinoEmisor == 1){

            converterMAC(nodo1[0], destino);

        }else if(nodoDestinoEmisor == 2){

            converterMAC(nodo2[0], destino);

        }else if(nodoDestinoEmisor == 3){

            converterMAC(nodo3[0], destino);

        }

        printf("Destino: %02X:%02X:%02X:%02X:%02X:%02X\n", destino[0], destino[1], destino[2], destino[3], destino[4], destino[5]);

        BYTE mensaje[31];
        printf("Ingresar mensaje, maximo 30 caracteres\n");
        getchar();
        fgets((char*)mensaje, sizeof(mensaje), stdin);
        int largoMensaje = contNoNullChar(mensaje);
        printf("Mensaje: %s", mensaje);
        printf("Largo mensaje: %i\n", largoMensaje);
        int largoOurProtocol = largoMensaje + 3;
        BYTE frameOurProtocol[largoOurProtocol];

        createOurProtocolFrame(0, largoMensaje, mensaje, frameOurProtocol);

        int largoEthernetProtocol = largoOurProtocol + 19;

        if(largoOurProtocol < 15){

            largoEthernetProtocol = 15 + 19;

        }

        
        BYTE ethernetProtocol[largoEthernetProtocol];

        createEthernetFrame(destino, origen, largoOurProtocol, frameOurProtocol, ethernetProtocol);

        largoSlipProtocol = contLengthSLIP(ethernetProtocol, largoEthernetProtocol);

        printf("largoOurProtocol: %i\n", largoOurProtocol);
        printf("largoEthernetProtocol: %i\n", largoEthernetProtocol);
        printf("largoSlipProtocol: %i\n", largoSlipProtocol);

        generateFrameSLIP(ethernetProtocol, slipProtocol, largoEthernetProtocol);

        // for(int i = 0; i < largoOurProtocol; i++){

        //     printf("frameOurProtocol = %i\n", frameOurProtocol[i]);

        // }
        
        // for(int i = 0; i < largoEthernetProtocol; i++){

        //     printf("ethernetProtocol = %i\n", ethernetProtocol[i]);

        // }

        for(int i = 0; i < largoSlipProtocol; i++){

            printf("slipProtocol = %i\n", slipProtocol[i]);

        }

        startTransmission();//Se comienza la transmisión]

        while(transmissionStarted){//Delay entre transmisiones 
            
            printf("Comenzo la transmision\n");
            delay(10000);

        }
    
    
        emisor = false;    

    }



    return 0;

}

void leerArchivo(const char* rutaArchivo, char* variables[4]) {
    int numeroLinea = 1; // Comenzar desde la línea 1
    FILE* archivo = fopen(rutaArchivo, "r");
    if(archivo == NULL){
        printf("Error al abrir el archivo\n");
        return;
    }

    char Linea[256];
    int x = 0;
    while(fgets(Linea, sizeof(Linea), archivo)){
        // Verificar si el número de línea actual coincide con las líneas deseadas
        //printf("Linea(%d): %s",numeroLinea, Linea);
        if (numeroLinea == 2 || numeroLinea == 5 || numeroLinea == 8 || numeroLinea == 11) {
            // Hacer una copia de la línea y asignarla a la variable correspondiente
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

int contNoNullChar(BYTE* array){

    int contador = 0;

    while(*array != '\0'){

        contador++;
        array++;

    }

    return contador;

}

void createOurProtocolFrame(int cmd, int longitud, BYTE* data, BYTE* frame){

    int fcs = 0;

    frame[0] = cmd | (longitud << 3);

    for(int i = 0; i < longitud; i++){

        frame[1 + i] = data[i];

    }

    for(int i = 0; i < longitud + 1; i++){

        fcs += contFCSByte(frame[i]);

    } 

    frame[longitud + 1] = fcs & 0xFF;
    frame[longitud + 2] = fcs >> 8;

}

void createEthernetFrame(BYTE* destino, BYTE* origen, int longitud, BYTE* data, BYTE* frame){

    long fcs = 0;

    for(int i = 0; i < 6; i++){

        frame[i] = destino[i];
        frame[i + 6] = origen[i];

    }

    frame[12] = 3; // TTL

    frame[13] = longitud & 0xFF;
    frame[14] = longitud >> 8;

    for(int i = 0; i < longitud; i++){

        frame[15 + i] = data[i];

    }

    if(longitud < 15){

        for(int i = 0; i < 15 - longitud; i++){

            frame[15 + i + longitud] = 0;

        }

        longitud = 15;

    }

    frame[13] = longitud & 0xFF;
    frame[14] = longitud >> 8;

    for(int i = 0; i < 15 + longitud; i++){

        fcs += contFCSByte(frame[i]);

    }

    frame[15 + longitud] = fcs & 0xFF;
    frame[16 + longitud] = fcs >> 8;
    frame[17 + longitud] = fcs >> 16;
    frame[18 + longitud] = fcs >> 24;

}

int contFCSByte(BYTE byte){

    int contador = 0;

    for(int i = 0; i < 8; i++){

        if(byte & (1 << i)){

            contador ++;

        }

    }

    return contador;

}

int contLengthSLIP(BYTE* frame, int lenghtFrame){//Cuenta los bits del arreglo después de aplicar SLIP

    int cont = 2; 
    int i;

    for(int n = 0; n < lenghtFrame; n++){ //Se itera por cada BYTE del frame 
    
        i = frame[n];

        if(i == 192){ //Se cuenta dos en caso de que el BYTE sea igual a 192

            cont+=2;

        }else if(i==219){ //Se cuenta dos en caso de que el BYTE sea igual a 219


            cont+=2;

        }else{

            cont+=1; //Se agrega 1 en todos los demás casos

        }

    }

    return cont;

}

void generateFrameSLIP(BYTE* frame, BYTE* frameSLIP, int lenghtFrame){//Genera el frame de SLIP

    int i;
    int cont = 1;

    frameSLIP[0] = 192; //Agrega BYTE de inicio

    for(int n = 0; n < lenghtFrame; n++){//Se itera por cada BYTE del frame 

        i = frame[n];

        if(i == 192){//Se reemplaza el 192

            frameSLIP[cont] = 219;
            cont++;
            frameSLIP[cont] = 220;
            cont++;

        }else if(i==219){//Se reemplaza el 219

            frameSLIP[cont] = 219;
            cont++;
            frameSLIP[cont] = 221;
            cont++;

        }else{

            frameSLIP[cont] = i;
            cont++; 

        }

    }

    frameSLIP[cont] = 192; //Agrega BYTE de final

}

void sendFrame(void){   //Función de envío de información

    if(transmissionStarted){
  
        digitalWrite(PinOut, (slipProtocol[nbytes] >> (7 - nbits)) & 0x01);
        printf("%i", slipProtocol[nbytes] >> (7 - nbits) & 0x01);
       
        nbits++; //Actualiza contador de bits
        
        if(nbits == 8){//Actualiza el número de Frame
            
            nbits = 0;
            printf("  %d  %c\n", slipProtocol[nbytes], slipProtocol[nbytes]);
            nbytes++;

            //Finaliza la comunicación
            if(nbytes == largoSlipProtocol){
                transmissionStarted = false;
                nbytes = 0;
            }

        }

    }

}

void startTransmission(){   //Cambia el condicional que controla la transmisión de información

    transmissionStarted = true;

}
