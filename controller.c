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
    T1_IDLE,
    T1_LIGADO,
    T1_QUEUE
} estadosT1;

//Estados do Tapete 2
typedef enum{
    T2_IDLE,
    T2_LIGADO,
    T2_FLUSH,
    T2_RECEIVE
} estadosT2;

//Estados do Tapete 3
typedef enum{
    T3_IDLE,
    T3_LIGADO,
    T3_FLUSH,
    T3_RECEIVE
} estadosT3;

//Estados do Tapete 4
typedef enum{
    T4_IDLE,
    T4_LIGADO,
    T4_QUEUE
} estadosT4; 

//Estados Pusher 1
typedef enum{
    P1_IDLE,
    P1_MUDAR,
    P1_PUSH,
    P1_PULL
}estadosP1;

//Estados Pusher 2
typedef enum{
    P2_IDLE,
    P2_MUDAR,
    P2_PUSH,
    P2_PULL
}estadosP2;

//Estados Emitters
typedef enum{
    EM_ON,
    EM_OFF
} estadosEM;

//Estados para piscar LWAIT
typedef enum{
    lwaitON, lwaitOFF
} estadosBLINK;

// Declara variáveis para calculo do tempo de ciclo
uint64_t start_time=0, end_time=0, cycle_time=0;

// Declara temporizadores 
timerBlock timerBLINK, timerFLUSH;


//Estados iniciais
estadosBLINK blink_state = lwaitOFF;
estadosT1 t1_state = T1_IDLE;
estadosT4 t4_state = T4_IDLE;
estadosT2 t2_state = T2_IDLE;
estadosT3 t3_state = T3_IDLE;
estadosP1 p1_state = P1_IDLE;
estadosP2 p2_state = P2_IDLE;
estadosEM em_state = EM_OFF;

//Declara funções para os timers
void update_timers();
void start_timer(timerBlock* t);
void stop_timer (timerBlock* t); 

//Declara funções para flancos
bool flanco_ascendente(bool sensor, bool *previous);
bool flanco_descendente (bool sensor, bool *previous);
void ler_flancos();

bool *previousSTART; //pointer para o estado anterior de START
bool flancoSTART;

bool *previousSTOP; //pointer para o estado anterior de STOP
bool flancoSTOP;

bool *previousST2; //pointer para o estado anterior de ST2
bool flancoST2;

bool *previousST3; //pointer para o estado anterior de ST3
bool flancoST3;

bool *previousSTR1;
bool flancoSTR1;

bool *previousSTR2;
bool flancoSTR2;

int main() {                

    previousSTOP = (bool*) malloc(sizeof(bool));
    previousSTART = (bool*) malloc(sizeof(bool));
    previousST2 = (bool*) malloc(sizeof(bool));
    previousST3 = (bool*) malloc(sizeof(bool));
    previousSTR1 = (bool*) malloc(sizeof(bool));
    previousSTR2 = (bool*) malloc(sizeof(bool));

    *previousSTOP = 1; //botão STOP tem lógica negada -> está a 1 desligado
    *previousSTART = 0; //inicializado desligado - lógica normal
    *previousST2 = 0; //inicializa a 0
    *previousST3 = 0; //inicializa a 0
    *previousSTR1 = 0; //inicializa a 0
    *previousSTR2 = 0; //inicializa a 0



    while(1){
        
        update_timers();
        read_inputs();
        ler_flancos();

        if (flancoSTOP) printf("\nCLICASTE NO STOP WOW\n");           
        if (flancoSTART)  printf("\nCLICASTE NO START WOW\n");  //botão START flanco

        //-------------TRANSIÇÃO DE ESTADOS----------

        //TAPETE 1
        switch (t1_state)
        {
        case T1_IDLE:
            if (flancoSTART) t1_state = T1_LIGADO;
            break;
        
        case T1_LIGADO:
            if (flancoSTOP) start_timer(&timerFLUSH);
            if (SV1 != 0) start_timer(&timerFLUSH);
            if (timerFLUSH.time >= 10000) t1_state = T1_IDLE;
            if (SPR1 != 1) t1_state = T1_QUEUE;
            break;
        
        case T1_QUEUE:
            if (SPR1 == 1) t1_state = T1_LIGADO;
            break;
        }

        //TAPETE 4
        switch (t4_state)
        {
        case T4_IDLE:
            if (flancoSTART) t4_state = T4_LIGADO;
            break;
        
        case T4_LIGADO:
            if (flancoSTOP) start_timer(&timerFLUSH);
            if (SV2 != 0) start_timer(&timerFLUSH);
            if (timerFLUSH.time >= 10000) t2_state = T4_IDLE;
            if (SPR2 != 1) t2_state = T4_QUEUE;
            break;
        
        case T4_QUEUE:
            if (SPR2 == 1) t4_state = T4_LIGADO;
            break;
        }

        //TAPETE 2
        switch (t2_state)
        {
        case T2_IDLE:
            if (SV1 != 0) t2_state = T2_LIGADO;
            break;
        
        case T2_LIGADO:
            if (t1_state == T1_IDLE && t4_state == T4_IDLE) t2_state = T2_FLUSH;
            if (PE2 == 1) t2_state = T2_RECEIVE;
            if (SPR1 == 0 || flancoST2) t2_state = T2_IDLE;
            break;

        case T2_RECEIVE:
            if (SPE2 == 0) t2_state = T2_LIGADO;
            break;

        case T2_FLUSH:
            if (timerFLUSH.time >= 25000) t2_state = T2_IDLE;
            break;
        }

        //TAPETE 3
        switch (t3_state)
        {
        case T3_IDLE:
            if (SV2 != 0) t3_state = T3_LIGADO;
            break;
        
        case T3_LIGADO:
            if (t1_state == T1_IDLE && t4_state == T4_IDLE) t3_state = T3_FLUSH;
            if (PE1 == 1) t3_state = T3_RECEIVE;
            if (SPR2 == 0 || flancoST3) t3_state = T3_IDLE;
            break;

        case T3_RECEIVE:
            if (SPE1 == 0) t3_state = T3_LIGADO;
            break;

        case T3_FLUSH:
            if (timerFLUSH.time >= 25000) t3_state = T3_IDLE;
            break;
        }
        //if (t2_state == IDLE && t3_state == IDLE) stop_timer(&timerFLUSH);
    
        //PUSHER 1
        switch (p1_state)
        {
        case P1_IDLE:
            if(SV1 == 4) p1_state = P1_MUDAR;
            break;
        
        case P1_MUDAR: 
            if (flancoSTR1) p1_state = P1_PUSH;
            break;

        case P1_PUSH:
            if (SPE1 == 1) p1_state = P1_PULL;
            break;
    
        case P1_PULL:
            if (SPR1 == 1) p1_state = P1_IDLE;
            break;
        }

        //PUSHER 2
        switch (p2_state)
        {
        case P2_IDLE:
            if(SV2 == 1) p2_state = P2_MUDAR;
            break;
        
        case P2_MUDAR: 
            if (flancoSTR2) p2_state = P2_PUSH;
            break;

        case P2_PUSH:
            if (SPE2 == 1) p2_state = P2_PULL;
            break;
    
        case P2_PULL:
            if (SPR2 == 1) p2_state = P2_IDLE;
            break;
        }

        //EMITTERS
        switch (em_state)
        {
        case EM_OFF:
            if (flancoSTART) em_state = EM_ON;
            break;
        
        case EM_ON:
            if (flancoSTOP) em_state = EM_OFF;
            break;
        }

        //BLINK
        switch (blink_state){    //NEEDS FINISHING
    
            case lwaitON:
    
            printf("Estado: LWAITON\nTimerBLINK:%d\nTimerFLUSH:%d\n\n",(int)timerBLINK.time,(int)timerFLUSH.time);
            
            if (timerFLUSH.time >= 25000) { //ACABAR O MODO A_PARAR
                blink_state = lwaitOFF;
                stop_timer(&timerBLINK);
                stop_timer(&timerFLUSH);
                break;
            }

            if (timerBLINK.time >= 1000){
            blink_state = lwaitOFF;
            start_timer(&timerBLINK); 
            }
            break;
    
            case lwaitOFF:         

            printf("Estado: LWAITOFF\nTimerBLINK:%d\nTimer2:%d\n\n",(int)timerBLINK.time,(int)timerFLUSH.time);

            if (flancoSTOP){ //começa a piscar se o STOP for largado (lógica negada)
                start_timer(&timerFLUSH);
                start_timer(&timerBLINK);
                blink_state = lwaitON;
                break;
            }   
            if (timerBLINK.time>1000){ //tempo em milisegundos                     
                blink_state = lwaitON;
                start_timer(&timerBLINK);
                break;
            } 
        }



        //--------OUTPUTS----------

        //TAPETE 1
        switch (t1_state)
        {
        case T1_IDLE:
            T1A=0;
            break;
        
        case T1_LIGADO:
            T1A=1;
            break;
        
        case T1_QUEUE:
            T1A=0;
            break;
        }

        //TAPETE 4
        switch (t4_state)
        {
        case T4_IDLE:
            T4A=0;
            break;
        
        case T4_LIGADO:
            T4A=1;
            break;
        
        case T4_QUEUE:
            T4A=0;
            break;
        }

        //TAPETE 2
        switch (t2_state)
        {
        case T2_IDLE:
            T2A=0;
            break;
        
        case T2_LIGADO:
            T2A=1;
            break;

        case T2_RECEIVE:
            T2A=0;
            break;

        case T2_FLUSH:
            T2A=1;
            break;
        }

        //TAPETE 3
        switch (t3_state)
        {
        case T3_IDLE:
            T3A=0;
            break;
        
        case T3_LIGADO:
            T3A=1;
            break;

        case T3_RECEIVE:
            T3A=0;
            break;

        case T3_FLUSH:
            T3A=1;
            break;
        }
        //if (t2_state == IDLE && t3_state == IDLE) stop_timer(&timerFLUSH);
    
        //PUSHER 1
        switch (p1_state)
        {
        case P1_IDLE:
            PE1=0;
            PR1=0;
            break;
        
        case P1_MUDAR: 
            PE1=0;
            PR1=0;
            break;

        case P1_PUSH:
            PE1=1;
            PR1=0;
            break;
    
        case P1_PULL:
            PE1=0;
            PR1=1;
            break;
        }

        //PUSHER 2
        switch (p2_state)
        {
        case P2_IDLE:
            PE2=0;
            PR2=0;
            break;
        
        case P2_MUDAR: 
            PE2=0;
            PR2=0;
            break;

        case P2_PUSH:
            PE2=1;
            PR2=0;
            break;
    
        case P2_PULL:
            PE2=0;
            PR2=1;
            break;
        }
        
        //EMITTERS
        switch (em_state)
        {
        case EM_OFF:
            E1=0;
            E2=0;
            break;
        
        case EM_ON:
            E1=1;
            E2=1;
            break;
        }

        //OUTPUT BLINK
        switch (blink_state){
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

//-------Funções timers---------

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
    if (timerFLUSH.on) timerFLUSH.time = timerFLUSH.time + cycle_time;
}



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

void ler_flancos(){

    flancoSTART = flanco_descendente(START,previousSTART); 
    flancoSTOP = flanco_ascendente(STOP,previousSTOP);
    flancoST2 = flanco_descendente(ST2,previousST2);
    flancoST3 = flanco_descendente(ST3,previousST3);
    flancoSTR1 = flanco_descendente(STR1,previousSTR1);
    flancoSTR2 = flanco_descendente(STR2,previousSTR2);
}
