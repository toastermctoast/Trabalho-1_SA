// ---------- Máquina de estados -----------

#include "IO.c"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <modbus/modbus.h>

//TEMPORIZADOR
typedef struct {
    bool on; 
    uint64_t time; 
} timerBlock;

//Estados do Tapete 1
typedef enum {
    
} estadosT1;

//Estados do Tapete 2
typedef enum{
    
} estadosT2;

//Estados do Tapete 3
typedef enum{
    
} estadosT3;

//Estados do Tapete 3
typedef enum{
    
} estadosT4;

typedef enum{
    lwaitON, lwaitOFF
} estadosBLINK;


// Declara variáveis para calculo do tempo de ciclo
uint64_t start_time=0, end_time=0, cycle_time=0;

// Declara temporizadores 
timerBlock timer1, timer2;


//Estados iniciais
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

    while(1){
        
        update_timers();
        read_inputs();

        /*-------------TRANSIÇÃO DE ESTADOS----------*/

        //BLINK
        switch (blinkState){
    
            case lwaitON:

            timer = 0;
            if (PARADO) {
                blinkState = lwaitOFF;
                break;
            }

            if (timer>1)
            blinkState = lwaitOFF;
            break;
    
            case lwaitOFF:

            timer = 0;
            if (flanco_descendente(STOP)){ 
                blinkState = lwaitON;
                break;
            }   
            if (timer>1){                       //fazer update ao timer??
                blinkState = lwaitON;
                break;
            } 
        }



        /*--------OUTPUTS----------*/


        //OUTPUT BLINK
        switch (blinkState){
        case lwaitON:
            LWAIT = 1;
            break;
    
        case lwaitOFF:
            LWAIT = 0;
            break;
        }

        write_outputs();
    
    }

}

void update_timers() {

 // Calcula o tempo de ciclo
    end_time = get_time();

    if (start_time == 0)
        cycle_time = 0;
    else
        cycle_time = end_time – start_time;

    // o fim do ciclo atual é o inicio do próximo 
    start_time = end_time;

// Atualiza temporizadores
    if (timer1.on) timer1.time = timer1.time + cycle_time;
    if (timer2.on) timer2.time = timer2.time + cycle_time;
}
