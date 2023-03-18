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

/*
//Estados do Tapete 1
typedef enum {
    
} estadosT1;

//Estados do Tapete 2
typedef enum{
    
} estadosT2;

//Estados do Tapete 3
typedef enum{
    
} estadosT3;

//Estados do Tapete 4
typedef enum{
    
} estadosT4; */

typedef enum{
    lwaitON, lwaitOFF
} estadosBLINK;


// Declara variáveis para calculo do tempo de ciclo
uint64_t start_time=0, end_time=0, cycle_time=0;

// Declara temporizadores 
timerBlock timerBLINK, timer2;


//Estados iniciais
estadosBLINK blinkState = lwaitOFF;

//Declara funções para os timers
void update_timers();
void start_timer(timerBlock* t);
void stop_timer (timerBlock* t); 

bool *previousSTART; //pointer para o estado anterior de START
bool *previousSTOP; //pointer para o estado anterior de STOP

//-------Funções flanco---------

bool flanco_ascendente(bool sensor, bool *previous){ //dás o sensor e o seu valor anterior
    bool flanco;
    if (sensor > *previous) flanco = true;
    else flanco = false;

    *previous = sensor;
    return flanco;
}

bool flanco_descendente (bool sensor, bool *previous){ //dás o sensor e o valor anterior
    bool flanco;
    if (sensor < *previous){
        flanco = true;
    }
    else flanco = false;

    *previous = sensor;
    return flanco;
}


int main() {


    previousSTOP = (bool*) malloc(sizeof(bool));
    previousSTART = (bool*) malloc(sizeof(bool));
    *previousSTOP = 1; //botão STOP tem lógica negada -> está a 1 desligado
    *previousSTART = 0; //inicializado desligado - lógica normal


    while(1){
        
        update_timers();
        //printf("Timer atualizado com sucesso!\n");
        read_inputs();
        //printf("Entradas lidas com sucesso!\n");

        if (previousSTOP == NULL) printf("We got a problem with the stop pointer here chief\n");

        //printf ("PREVIOUS STOP VALUE=%d\n",*previousSTOP);

        if (flanco_ascendente(STOP,previousSTOP)) printf("\nCLICASTE NO STOP WOW\n");           
        if (flanco_descendente(START,previousSTART))  printf("\nCLICASTE NO START WOW\n");  //botão START flanco git

        //-------------TRANSIÇÃO DE ESTADOS----------

/*
        //BLINK
        switch (blinkState){    //NEEDS CORRECTING
    
            case lwaitON:
    
            start_timer(&timerBLINK);
            if (ACABAR O MODO A_PARAR) { //ACABAR O MODO A_PARAR
                blinkState = lwaitOFF;
                stop_timer(&timerBLINK);
                break;
            }

            if (timer2.time > 1)
            blinkState = lwaitOFF;
            break;
    
            case lwaitOFF:         

            start_timer(&timerBLINK);

            if (flanco_descendente(STOP,previousSTOP)){ //começa a piscar?
                blinkState = lwaitON;
                break;
            }   
            if (timerBLINK.time>1){                     
                blinkState = lwaitON;
                break;
            } 
        }



        //--------OUTPUTS----------


        //OUTPUT BLINK
        switch (blinkState){
        case lwaitON:
            LWAIT = 1;
            break;
    
        case lwaitOFF:
            LWAIT = 0;
            break;
        } */

        write_outputs();
    
    }

}
void start_timer(timerBlock* t) {   
    t->on = true;
    t->time = 0;
}

void stop_timer(timerBlock* t) { 
    t->on = false;
    t->time = 0;
}

void update_timers() {

 // Calcula o tempo de ciclo
    end_time = get_time();

    if (start_time == 0)
        cycle_time = 0;
    else
        cycle_time = end_time - start_time;

    // o fim do ciclo atual é o inicio do próximo 
    start_time = end_time;

// Atualiza temporizadores
    if (timerBLINK.on) timerBLINK.time = timerBLINK.time + cycle_time;
    if (timer2.on) timer2.time = timer2.time + cycle_time;
}

