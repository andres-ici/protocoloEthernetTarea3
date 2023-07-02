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

//Funciones de recepcion de datos

void leerBit();
bool compararMAC(BYTE MACdestino[6],const char *MAC);

//Variables

bool emisor;
int nodo;
char* varTxt[4];
int largoSlipProtocol;
int PinOut;
int PinIn;

BYTE slipProtocol[100];
volatile int nbits = 0; //Cuenta la cantidad de bits recibidos
volatile int nbytes = 0; //Cuenta el número de Byte enviados

volatile int j = 0; //Cuenta la cantidad de bits hasta formar un byte
BYTE c = 0; //Se forma y se guarda el byte actual recibido
BYTE d = 0; //Representa el valor previo de “c”
bool transmissionStarted2 = false; //Indica el estado de la transmisión 
bool finished = false; //Finaliza el programa 

BYTE C0 = 192; //Guarda el valor de decimal de C0 en C0
BYTE DB = 219; //Guarda el valor de decimal de DB en DB
BYTE DC = 220; //Guarda el valor de decimal de DC en DC
BYTE DD = 221; //Guarda el valor de decimal de DD en DD

BYTE msg[300]; //Se guarda todo el mensaje recibido excepto los bytes de inicio y final 
//int index = 0; //Desface requerido para guardar el byte en caso de encontrar un DBDC y/o DBDD
int B = 0; //Cantidad de bytes recibidos
int FCScalculadoCapaEthernet = 0; //Se guarda el valor del FCS calculada del receptor
int FCScalculadoCapaPropio = 0; //Se guarda el valor del FCS calculada del receptor

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
    PinIn = atoi(varTxt[1]);
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

    //Codigo para el Receptor 

    pinMode(PinIn, INPUT);
    printf("Esperando...\n");

    if(wiringPiISR(Clock, INT_EDGE_RISING, &leerBit) < 0){ //Interrumpe el programa para leer el mensaje
    printf("Unable to start interrupt function\n");
    }

    while(!finished); 
    printf("\n");

    BYTE MACdestino[6] = {msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]};
    BYTE MACorigen[6] = {msg[6], msg[7], msg[8], msg[9], msg[10], msg[11]};

    printf("MACdestino: %02X:%02X:%02X:%02X:%02X:%02X\n", msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]);
    printf("MACorigen: %02X:%02X:%02X:%02X:%02X:%02X\n", msg[6], msg[7], msg[8], msg[9], msg[10], msg[11]);

    if(compararMAC(MACdestino, MAC)){ //Compara si la MAC del destino es igual a la MAC del receptor
        
        int largoCapaEthernet = msg[13] + (msg[14] << 8);
        int FCScapaEthernet = 0;
        int x = 0;
        for(int i = (16 + largoCapaEthernet); i <= (16 + largoCapaEthernet) + 4; i++){
        
            FCScapaEthernet =  FCScapaEthernet + (msg[i] << x*8);
            x++;
        }
        
        for(int i = 0; i <= (15 + largoCapaEthernet); i++){ //Calcula el FCS 
            for(int x = 0; x < 8; x++){
                FCScalculadoCapaEthernet = FCScalculadoCapaEthernet + (int)((msg[i] >> x) & 0x01);
            }
        }


       
        // Data + Relleno: [15, 15 + Longitud]

        // msg[15]       = |L|L|L|L|L|C|C|C|
        // msg[16 al 46] = |D|D|D|D|D|D|D|D|
        // msg[47]       = |F|F|F|F|F|F|F|F|
        // msg[48]       = |_|_|_|_|_|_|_|F|


        // Destribucion Capa Propia
        //CMD: (msg[15] & 0x07) 
        //Longitud: (msg[15] & 0xF8) >> 3
        //Dato: msg[15] ... msg[15 + longitud]
        //FCS: msg[16 + longitud] | (msg[17 + longitud] & 0x01) << 8 


        int cmdCapaPropio = msg[15] & 0x07;
        int largoCapaPropio = (msg[15] & 0xF8) >> 3;
        int FCScapaPropio = msg[16 + largoCapaPropio] | (msg[17 + largoCapaPropio] & 0x01) << 8;


        for(int i = 15; i <= (15 + largoCapaPropio); i++){ //Calcula el FCS 
            for(int x = 0; x < 8; x++){
                FCScalculadoCapaPropio = FCScalculadoCapaPropio + (int)((msg[i] >> x) & 0x01);
            }
        }
       



        printf("\n\nCapa_Ethernet\n");
        printf("M.A.C Destino:");
        for(int i = 0 ; i <= 5; i++){
            printf("%c",msg[i]);
        }
        printf("\n");
        printf("M.A.C Origen:");
        for(int i = 6 ; i <= 11; i++){
            printf("%c",msg[i]);
        }
        printf("\n");
        printf("TTL:%d\n",msg[12]);
        printf("largoCapaEthernet: %d\n",largoCapaEthernet);
        printf("FCScapaEthernet: %d\n",FCScapaEthernet);
        printf("FCScalculadoCapaEthernet: %d\n",FCScalculadoCapaEthernet);


        printf("\n\nCapa_Propia\n");
        printf("cmdCapaPropia: %d\n",cmdCapaPropio);
        printf("largoCapaPropia: %d\n",largoCapaPropio);
        printf("FCScapaPropio: %d\n",FCScapaPropio);
        printf("FCScalculadoCapaPropio: %d\n",FCScalculadoCapaPropio);

        printf("\n\nMensaje_Recibido\n");
        for(int i = 16; i < (16 + largoCapaPropio);i++){
            printf("%c",msg[i]);
        }
        printf("\n\n\n\n");



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

void leerBit(){
    int largo; //Se inicializa el largo del mensaje 
    int cmd; //Se inicializa el comando del mensaje
    int comienzoData = 1; //Se inicializa el comienzo de la data
    int MACdestino = 0;
    bool bit = digitalRead(PinIn); //Se leen los bits del pin
    int index = 0;
  
    c = (c << 1) | bit; //Se guardan los bits en “c”

    if(transmissionStarted2 == false){
        if(c == C0){ //Si se detecta un C0 se inicia la transmisión y se resetean las variables
            printf("inicio\n");
            j = 0;
            B = 0;
            c = 0;
            d = 0;  
            index = 0;
            transmissionStarted2 = true;
        }
    }else{
        printf ("%d", bit);
        j++; 

        if(j == 8){
            j = 0;   

            msg[B-index] = c; //Se guarda el byte de “c” en msg[] 

            if(c == C0){ //Se activa con el segundo C0 es decir con el byte de final
                finished = true;
                for(int i = 0; i < 100; i++){
            
                    printf("MSG[%i]: %i\n", i, msg[i]);

                }
                return;
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
