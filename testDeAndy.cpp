#include <wiringPi.h>
#include <stdio.h>
#include <cstring>
#include <cstdio>
#include <math.h>

#define BYTE unsigned char

//Functions
void leerArchivo(const char* rutaArchivo, char* variables[4]);
void converterMAC(const char* MAC, BYTE* destino);
void createEthernetFrame(BYTE* destino, BYTE* origen, int longitud, BYTE* data, BYTE* frame);
void leerBit(int pinIn,  BYTE* MAC, BYTE* msg[300]);
bool compararMAC(BYTE MACdestino[6],const char *MAC);
void createOurProtocolFrame(int cmd, int longitud, BYTE* data, BYTE* frame);
int contNoNullChar(BYTE* array);
int contFCSByte(BYTE byte);
int contLengthSLIP(BYTE* frame, int lenghtFrame);
void generateFrameSLIP(BYTE* frame, BYTE* frameSLIP, int lenghtFrame);

char* varTxt[4];
bool emisor = false;
int nodo;

volatile int nbits = 0; //Cuenta la cantidad de bits recibidos
volatile int nbytes = 0; //Cuenta el número de Byte enviados
bool transmissionStarted = false; //Condicional para el comienzo de la transmisión
BYTE slipProtocol[100];

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

BYTE* msg[300]; //Se guarda todo el mensaje recibido excepto los bytes de inicio y final 
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


int main(int argc, char* argv[]){
    // Print the number of command line arguments entered

    printf("You have entered %d arguments:\n", argc);

    // Print each command line argument entered
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
    if (strcmp(argv[1], "1") == 0) {
        printf("Nodo 1\n");
        nodo = 1;
        const char* rutaArchivo = "nodo_1.txt"; 
        leerArchivo(rutaArchivo,varTxt);
    } else if (strcmp(argv[1], "2") == 0) {
        printf("Nodo 2\n");
        nodo = 2;
        const char* rutaArchivo = "nodo_2.txt"; 
        leerArchivo(rutaArchivo,varTxt);
    } else if (strcmp(argv[1], "3") == 0) {
        printf("Nodo 3\n");
        nodo = 3;
        const char* rutaArchivo = "nodo_3.txt"; 
        leerArchivo(rutaArchivo,varTxt);
    } else {
        // Print an error message if the node is not recognized
        printf("Error con el Nodo\n");
    }

    char *MAC = varTxt[0]; 
    int PinIn = *varTxt[1] - 48;
    int PinOut = *varTxt[2] - 48;
    int Clock = *varTxt[3] - 48;

    printf("M.A.C: %s",MAC);
    printf("PinIn: %d\n",PinIn);
    printf("PintOut: %d\n",PinOut);
    printf("Clock: %d\n",Clock);

    if(wiringPiSetup() == -1){ //Verificar correcto funcionamiento de Raspberry Pi

        printf("Error");

    }

    //CONFIGURA INTERRUPCION PIN CLOCK (PUENTEADO A PIN PWM)
    if(wiringPiISR(Clock, INT_EDGE_FALLING, &sendFrame) < 0){

        printf("Unable to start interrupt function\n");

    }

    pinMode(pinOut, OUTPUT); //Se define el pin como salida

    digitalWrite(pinOut, 0);

    if(emisor){
        printf("Nodo: %d, modo Emisor, M.A.C: %s",nodo, MAC);
    }else{
        printf("Nodo: %d, modo Receptor, M.A.C: %s",nodo, MAC);
    }

    BYTE origen[6];

    converterMAC(MAC, origen);

    printf("Origen: %02X:%02X:%02X:%02X:%02X:%02X\n", origen[0], origen[1], origen[2], origen[3], origen[4], origen[5]);


   

    //              ____               _   _                           
    //             / ___|   ___     __| | (_)   __ _    ___            
    //   _____    | |      / _ \   / _` | | |  / _` |  / _ \     _____ 
    //  |_____|   | |___  | (_) | | (_| | | | | (_| | | (_) |   |_____|
    //             \____|  \___/   \__,_| |_|  \__, |  \___/           
    //                                         |___/                     

    // Destribucion Capa Ethernet 
    // Destino: [0,5]
    // Origen: [6,11]
    // TTL: [12]
    // Longitud: [13,14]
    // Data + Relleno: [15, 15 + Longitud]
    // FCS: [(15 + Longitud) + 1, (15 + Longitud) + 4]

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

        int largoSlipProtocol = contLengthSLIP(ethernetProtocol, largoEthernetProtocol);

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
            delay(2000);

        }
    
        emisor = false;    

    }
    //Codigo para el Receptor 

    // pinMode(PinIn, INPUT);
    // printf("Esperando...\n");

    // if(wiringPiSetup() == -1){
    // printf("Error wiringPiSetup");    
    // exit(1);}

    // if(wiringPiISR(Clock, INT_EDGE_RISING, &leerBit) < 0){ //Interrumpe el programa para leer el mensaje
    // printf("Unable to start interrupt function\n");
    // }


    // while(!finished); 
    // printf("\n");
    

    // BYTE MACdestino[6] = {msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]};
    // BYTE MACorigen[6] = {msg[6], msg[7], msg[8], msg[9], msg[10], msg[11]};
    // printf("Comparacion de mac:%d\n",compararMAC(MACdestino, MAC));

    // if(compararMAC(MACdestino, MAC)){ //Compara si la MAC del destino es igual a la MAC del receptor
        
    //     int largoCapaEthernet = msg[13] + (msg[14] << 8);
    //     printf("Longitud: %d\n",largoCapaEthernet);

        
    //     int FCScapaEthernet = 0;
    //     int x = 0;
    //     for(int i = (16 + largoCapaEthernet); i <= (16 + largoCapaEthernet) + 4; i++){
        
    //         FCScapaEthernet =  FCScapaEthernet + (msg[i] << x*8);
    //         x++;
    //     }
        
    //     printf("FCS: %d\n",FCScapaEthernet);

        

        

    // }else{
    //     msg[12] = msg[12] - 1; //Resta al TTL

    //     if(msg[12] == 0){ //No se renvia el mensaje 
    //     printf("Mensaje descartado por TTL\n");
    //     printf("TTL:%d\n",msg[12]);

    //     }else{ //Se renvia el mensaje
    //     printf("Reenviando...\n");
    //     printf("TTL:%d\n",msg[12]);

    //     }



    // }


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

void converterMAC(const char* MAC, BYTE* destino){

    sscanf(MAC, "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX", &destino[0], &destino[1], &destino[2], &destino[3], &destino[4], &destino[5]);

}

// void leerBit(int pinIn,  BYTE* MAC, BYTE* msg[300]){
//   int largo; //Se inicializa el largo del mensaje 
//   int cmd; //Se inicializa el comando del mensaje
//   int comienzoData = 1; //Se inicializa el comienzo de la data
//   int MACdestino = 0;
//   bool bit = digitalRead(pinIn); //Se leen los bits del pin
  
//   c = (c << 1) | bit; //Se guardan los bits en “c”

//     if(transmissionStarted == false){
//         if(c == C0){ //Si se detecta un C0 se inicia la transmisión y se resetean las variables
//             printf("inicio\n");
//             j = 0;
//             B = 0;
//             c = 0;
//             d = 0;  
//             FCS = 0;
//             index = 0;
//             FCSEmisor = 0;
//             conteoDeDataEsperada = 0;
//             transmissionStarted = true;
//         }
//     }else{
//         printf ("%d", bit);
//         j++; 

//         if(j == 8){
//             j = 0;   

//             msg[B-index] = c; //Se guarda el byte de “c” en msg[] 

//             if(c == C0){ //Se activa con el segundo C0 es decir con el byte de final
//                 finished = true;
//                 return;
//             }else{
//                 printf("  %d  %c\n",c,c);
//             }


//             if(d == DB & c == DC){ //Detecta DBDC
//                 printf("Se Recibio un: DBDC\n");
                
//                 msg[B-index] = 0; //Transforma el DB en 0 
//                 index++;
//                 msg[B-index] = C0; //Transforma el DC en C0
//             }  
//             if(d == DB & c == DD){ //Detecta DBDD
//                 printf("Se Recibio un: DBDD\n");

//                 msg[B-index] = 0; //Transforma el DB en 0
//                 index++;
//                 msg[B-index] = DB; //Transforma el DD en DB
//             } 

//             d = c;  
//             B++;
//         }
//     }






// }

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

int contNoNullChar(BYTE* array){

    int contador = 0;

    while(*array != '\0'){

        contador++;
        array++;

    }

    return contador;

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
  
        digitalWrite(pinOut, (slipProtocol[nbytes] >> (7 - nbits)) & 0x01);
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
