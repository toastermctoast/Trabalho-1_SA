// ---------- Máquina de estados -----------

#include "IO.c"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <modbus/modbus.h>

//Estados da Máquina MODOS
typedef enum {
    PARADO,OPERAR,A_PARAR,WAIT,FLUSH
} estadosMODOS;

//Estados da máquina SEPARAR
typedef enum{
    IDLE,SV1_AZUL,SV2_AZUL,SV1_VERDE,SV2_VERDE
} estadosSEPARAR;

typedef enum{
    lwaitON, lwaitOFF
} estadosBLINK;

//Estados iniciais
estadosMODOS modo = PARADO;
estadosSEPARAR separarState = IDLE;
estadosBLINK blinkState = lwaitOFF;

//Funções flanco

bool flanco_ascendente(bool sensor){

    bool flanco, previous = 0;
    if (sensor == true && previous == false) flanco = true;
    else flanco = false;

    previous = sensor;
    return flanco;
}

bool flanco_descendente (bool sensor){
    bool flanco, previous = 0;
    if (sensor == false && previous == true) flanco = true;
    else flanco = false;

    previous = sensor;
    return flanco;
}


int main{

    //MODOS

    switch (modo)
    {
    case PARADO:
        if (flanco_descendente(START)){

            C_AZUIS = 0;
            C_VERDES = 0;
            
            modo = OPERAR;
        }
        break;
    
    case OPERAR:
        if (flanco_descendente(STOP) modo = A_PARAR;
        break;

    case A_PARAR:
        
        if (SV==0) modo = WAIT; //tem de ser flanco??
        break;

    case WAIT:

        while (SV=0){
            //start timer
            if (t>10s){
                modo = FLUSH;
                break;
            } 
        if (modo!=FLUSH){
            modo = A_PARAR;
            //restart timer
        } 
        break;

    case FLUSH:
        if (t>15s) modo = PARADO;
        break;
    }

    //SEPARAR

    switch (separarState){
    case IDLE:
        if (SV1 = 1) separarState = SV1_AZUL; 
        break;
    case T1_AZUL:
        break;
    case T1_VERDE:
        break;
    case T4_VERDE:
        break;
    case T4_AZUL:
        break;
    case MANTER_T2:
        break;
    case MANTER_T3:
        break;
    }

    //BLINK

    switch (blinkState)
    {
    case lwaitON:
        if (t>1s) blinkState = lwaitOFF;
        t = 0;
        break;
    
    case lwaitOFF:
        if (t>10s) blinkState = lwaitON;
        t=0;
        break;
    }

    //OUTPUTS MODOS

    switch (modo){

    case PARADO:
        LSTOP = 1;
        LWAIT = 0;
        LSTART = 0;
        E1 = 0,E2 = 0;
        PE1 = 0, PR1 = 0, PE2 = 0, PR2 = 0;
        T1A= 0,T2A = 0,T3A = 0,T4A = 0;
        break;
    
    case OPERAR:
        LSTART = 1;
        LWAIT = 0;
        LSTOP = 0;
        E1 = 1,E2 = 1;
        break;
    case A_PARAR:

        break;

    default:
        break;
    }

    //OUTPUT SEPARAR

    switch (separarState){
    case IDLE:              //falta verificar se estamos no estado PARADO
        T1A = 1;
        T4A = 1;
        break;
    case T1_AZUL:
        break;
    case T1_VERDE:
        break;
    case T4_VERDE:
        break;
    case T4_AZUL:
        break;
    case MANTER_T2:
        break;
    case MANTER_T3:
        T3A = 1;
        T2A = 0, T1A = 0;
        break;
    }


    //OUTPUT BLINK
    switch (blinkState)
    {
    case lwaitON:
        LWAIT = 1;
        break;
    
    case lwaitOFF:
        LWAIT = 0;
        break;
    }

}
