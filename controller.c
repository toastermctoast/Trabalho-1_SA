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
    T2_MANTER,
    T2_MUDAR,
    T2_READY,
    T2_PUSH,
    T2_PULL,
    T2_RECEIVE,
    T2_ST,
    T2_FLUSH
} estadosT2;

//Estados do Tapete 3
typedef enum{
    T3_IDLE,
    T3_MANTER,
    T3_MUDAR,
    T3_READY,
    T3_PUSH,
    T3_PULL,
    T3_RECEIVE,
    T3_ST,
    T3_FLUSH
} estadosT3;

//Estados do Tapete 4
typedef enum{
    T4_IDLE,
    T4_LIGADO,
    T4_QUEUE
} estadosT4; 

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

        printf("T1 :%d  T4:%d    T2:%d  T3:%d\n",t1_state,t4_state,t2_state,t3_state);

        //-------------TRANSIÇÃO DE ESTADOS----------

        //TAPETE 1
        switch (t1_state)
        {
        case T1_IDLE:
            if (flancoSTART) t1_state = T1_LIGADO;
            break;
        
        case T1_LIGADO:
            if (flancoSTOP) start_timer(&timerFLUSH);
            if (SV1 != 0) timerFLUSH.time=0;
            if (timerFLUSH.time >= 10000) t1_state = T1_IDLE;
            //if (SV1 != 0 && (t2_state == T2_RECEIVE || t2_state == T2_ST || t2_state == T2_PULL)) t1_state = T1_QUEUE; 
            if (SV1 != 0 && (t2_state != T2_MUDAR && t2_state != T2_MANTER)) t1_state = T1_QUEUE;
            break;
        
        case T1_QUEUE:
            //if (t2_state != T2_RECEIVE && t2_state != T2_ST && t2_state != T2_PULL) t1_state = T1_LIGADO;
            if (t2_state == T2_MUDAR || t2_state == T2_MANTER) t1_state = T1_LIGADO;
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
            if (SV2 != 0) timerFLUSH.time=0;
            if (timerFLUSH.time >= 10000) t4_state = T4_IDLE;
            //if (SV2 != 0 && (t3_state == T3_RECEIVE || t3_state == T3_ST || t3_state == T3_PULL)) t4_state = T4_QUEUE;
            if (SV2 != 0 && (t3_state != T3_MUDAR && t3_state != T3_MANTER)) t4_state = T4_QUEUE;
            break;
        
        case T4_QUEUE:
            //if (t3_state != T3_RECEIVE && t3_state != T3_ST && t3_state != T3_PULL) t4_state = T4_LIGADO; 
            if (t3_state == T3_MUDAR || t3_state == T3_MANTER) t4_state = T4_LIGADO;
            break;
        }

        //TAPETE 2
        switch (t2_state)
        {
        case T2_IDLE: 
            if (SV1 == 1) t2_state = T2_MANTER;
            if (SV1 == 4) t2_state = T2_MUDAR;
            if (t3_state == T3_MUDAR || t3_state == T3_READY) t2_state = T2_RECEIVE;
            if (t1_state == T1_IDLE && t4_state == T4_IDLE && flancoSTOP) t2_state = T2_FLUSH; //o flancoSTOP desaparece
            break;
        
        case T2_MANTER:
            if (flancoST2) t2_state = T2_IDLE; 
            break;

        case T2_MUDAR:
            if (flancoSTR1) t2_state = T2_READY;
            break;

        case T2_READY:
            if (t3_state == T3_RECEIVE) t2_state = T2_PUSH;
            break;

        case T2_PUSH:
            if (SPE1 == 1) t2_state = T2_PULL; 
            break;

        case T2_PULL:
            if (SPR1 == 1) t2_state = T2_IDLE;
            break;

        case T2_RECEIVE:
            if (PR2 == 1) t2_state = T2_ST;
            break;

        case T2_ST:
            if (flancoST2) t2_state = T2_IDLE;
            break;

        case T2_FLUSH:
            if (timerFLUSH.time >= 25000) t2_state = T2_IDLE;
            break;
        }

        //TAPETE 3
        switch (t3_state)
        {
        case T3_IDLE:
            if (SV2 == 4) t3_state = T3_MANTER;
            if (SV2 == 1) t3_state = T3_MUDAR;
            if (t2_state == T2_MUDAR || t2_state == T2_READY) t3_state = T3_RECEIVE;
            if (t1_state == T1_IDLE && t4_state == T4_IDLE && flancoSTOP) t3_state = T3_FLUSH;
            break;
        
        case T3_MANTER:
            if (flancoST3) t3_state = T3_IDLE;
            break;

        case T3_MUDAR:
            if (flancoSTR2 ) t3_state = T3_READY;
            break;

        case T3_READY:
            if(t2_state == T2_RECEIVE) t3_state = T3_PUSH;
            break;

        case T3_PUSH:
            if (SPE2 == 1) t3_state = T3_PULL; 
            break;

        case T3_PULL:
            if (SPR2 == 1) t3_state = T3_IDLE;
            break;

        case T3_RECEIVE:
            if (PR1 == 1) t3_state = T3_ST;
            break;

        case T3_ST:
            if (flancoST3) t3_state = T3_IDLE;
            break;

        case T3_FLUSH:
            if (timerFLUSH.time >= 25000) t3_state = T3_IDLE;
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

            if (flancoSTOP){ //começa a piscar se o STOP for largado 
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
        case T2_RECEIVE:
        case T2_READY:
            T2A=0;
            PE1=0;
            PR1=0;
            break;
        
        case T2_MANTER:
        case T2_MUDAR:
        case T2_ST:
        case T2_FLUSH:
            T2A=1;
            PE1=0;
            PR1=0;
            break;

        case T2_PUSH:
            T2A=0;
            PE1=1;
            PR1=0;
            break;
    
        case T2_PULL:
            T2A=0;
            PE1=0;
            PR1=1;
            break;
        }

        //TAPETE 3
        switch (t3_state)
        {
        case T3_IDLE:
        case T3_RECEIVE:
        case T3_READY:
            T3A=0;
            PE2=0;
            PR2=0;
            break;
        
        case T3_MANTER:
        case T3_MUDAR:
        case T3_ST:
        case T3_FLUSH:
            T3A=1;
            PE2=0;
            PR2=0;
            break;
        case T3_PUSH:
            T3A=0;
            PE2=1;
            PR2=0;
            break;
    
        case T3_PULL:
            T3A=0;
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
