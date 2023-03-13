// ----------------- IO.c
#undef MODBUS_DEBUG


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <modbus/modbus.h>


// SENSORS
bool START = 0;
bool STOP = 0;
bool STR1 = 0;
bool STR2 = 0;
bool SPE1 = 0;
bool SPR1 = 0;
bool SPE2 = 0;
bool SPR2 = 0;
bool ST2 = 0;
bool ST3 = 0;
uint16_t SV1 = 0;
uint16_t SV2 = 0;



// ACTUATORS
bool  LSTART = 0;
bool  LSTOP = 0;
bool  LWAIT = 0;
bool  E1 = 0;
bool  E2 = 0;
bool  T1A = 0;
bool  T2A = 0;
bool  T3A = 0;
bool  T4A = 0;
bool  PE1 = 0;
bool  PR1 = 0;
bool  PE2 = 0;
bool  PR2 = 0;
uint16_t AZUIS = 0;
uint16_t VERDES = 0;


/* DO NOT ALTER THE FOLLOWING CODE */


// Modbus variables
#define IP_ADDR "127.0.0.1"
#define PORT 5502
#define MAX_D_INPUTS 16
#define MAX_D_OUTPUTS 16
#define MAX_R_INPUTS 2
#define MAX_R_OUTPUTS 2

modbus_t *mb = NULL;
int modbus_result = 0;
uint8_t tab_d_inputs[MAX_D_INPUTS];
uint8_t tab_d_outputs[MAX_D_OUTPUTS];
uint16_t tab_r_inputs[MAX_R_INPUTS];
uint16_t tab_r_outputs[MAX_R_OUTPUTS];

// Modbus functions
int mb_init();
void mb_shutdown();
int mb_read_I_D();
int mb_write_Q_D();
int mb_read_I_R();
int mb_write_Q_R();

// Functions
int read_inputs();
int write_outputs();
void sleep_rel(uint64_t);
void sleep_abs(uint64_t);
uint64_t get_time();



// -- CONTROLLER INTERFACE ---

int read_inputs() {
	
	modbus_result = mb_read_I_D();
	modbus_result =  mb_read_I_R() || modbus_result;
	return modbus_result;
}

int write_outputs() {
	
	modbus_result = mb_write_Q_D();
	modbus_result = mb_write_Q_R() || modbus_result ;
	
	return modbus_result;
}

// ---- TIME -----

// Sleep relative number of milliseconds       
void sleep_rel(uint64_t milliseconds) {
  uint seconds = milliseconds / 1000;
  milliseconds -= seconds*1000;
  struct timespec period = {seconds,               //tv_sec;  /* seconds */
                            milliseconds*1000*1000 //tv_nsec; /* nanoseconds */
                           };
                           
  //struct timespec start, end;
  //clock_gettime(CLOCK_MONOTONIC, &start);                           
  clock_nanosleep(CLOCK_MONOTONIC, 0, &period, NULL);
  //clock_gettime(CLOCK_MONOTONIC, &end);                           
    
  //printf("sleep_rel(%d); slept for %ld seconds, %ld ms\n", 
  //        milliseconds,
  //        end.tv_sec - start.tv_sec, 
  //        (end.tv_nsec - start.tv_nsec)/1000/1000); 
}



// Sleep absolute number of milliseconds       
void sleep_abs(uint64_t milliseconds) {
  static struct timespec next_wakeup = {0, 0};
  
  uint seconds = milliseconds / 1000;
  milliseconds -= seconds*1000;
  
  // initialise next_wakeup if it is the first time this function is called
  if ((next_wakeup.tv_sec == 0) && (next_wakeup.tv_nsec == 0))
    clock_gettime(CLOCK_MONOTONIC, &next_wakeup);
  
  // determine next absolute time to wake up
  next_wakeup.tv_sec  += seconds;
  next_wakeup.tv_nsec += milliseconds * 1000 * 1000;
  if (next_wakeup.tv_nsec >= 1000 * 1000 * 1000) {
      next_wakeup.tv_nsec -= 1000 * 1000 * 1000;
      next_wakeup.tv_sec  += 1;
  }
      
                           
  //struct timespec start, end;
  //clock_gettime(CLOCK_MONOTONIC, &start);                           
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_wakeup, NULL);
  //clock_gettime(CLOCK_MONOTONIC, &end);                           
    
  //printf("sleep_abs(%d); slept for %ld seconds, %ld ms\n", 
  //        milliseconds,
  //        end.tv_sec - start.tv_sec, 
  //        (end.tv_nsec - start.tv_nsec)/1000/1000); 
}



// return time in milliseconds
uint64_t get_time(void) {
  struct timespec now;
  
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (uint64_t) (now.tv_sec*1000) + (now.tv_nsec/1000/1000);  
}
      



// ---- MODBUS ------

void mb_shutdown(void) {
  modbus_close(mb);
  modbus_free(mb);
}

int mb_init(void) {
  // create new structure
  if ((mb = modbus_new_tcp(IP_ADDR, PORT)) == NULL) return 1;
  
  // set UID
  modbus_set_slave(mb, 1);
  
  // setup clean_up function
  if (atexit(mb_shutdown) < 0) return 1;
  
  // establish connection  
  return modbus_connect(mb);
}


int mb_read_I_D(void) {
	
	/* Read input discretes */
	
	if (mb == NULL)  mb_init();
  
  	#ifdef MODBUS_DEBUG
	printf("Read Inputs\n");
	#endif
	
	modbus_result= modbus_read_input_bits(mb, 0, MAX_D_INPUTS, tab_d_inputs);
	
  
	if (modbus_result < 0) return modbus_result;
  
	/* MAPPING */
	START = tab_d_inputs[0];
	STOP = tab_d_inputs[1];
	STR1 = tab_d_inputs[2];
	STR2 = tab_d_inputs[3];
	SPE1 = tab_d_inputs[4];
	SPR1 = tab_d_inputs[5];
	SPE2 = tab_d_inputs[6];
	SPR2 = tab_d_inputs[7];
	ST2 = tab_d_inputs[8];
	ST3 = tab_d_inputs[9];
	  
  return 0;  
}



int mb_write_Q_D(void) {
  
  	/* Write coils */
	
	#ifdef MODBUS_DEBUG
	printf("Write Coils\n");
	#endif
	
    
	/* MAPPING */
	tab_d_outputs[0] = LSTART;
	tab_d_outputs[1] = LSTOP;
	tab_d_outputs[2] = LWAIT;
	tab_d_outputs[3] = E1;
	tab_d_outputs[4] = E2;
	tab_d_outputs[5] = T1A;
	tab_d_outputs[6] = T2A;
	tab_d_outputs[7] = T3A;
	tab_d_outputs[8] = T4A;
	tab_d_outputs[9] = PE1;
	tab_d_outputs[10] = PR1;
	tab_d_outputs[11] = PE2;
	tab_d_outputs[12] = PR2;
		
	if (mb == NULL)  mb_init();
	return modbus_write_bits(mb, 0, MAX_D_OUTPUTS, tab_d_outputs);
}

int mb_read_I_R(void) {
	
	/* Read input registers */
	
	if (mb == NULL)  mb_init();
  
  	#ifdef MODBUS_DEBUG
	printf("Read Input Registers\n");
	#endif
	
	modbus_result= modbus_read_input_registers(mb, 0, MAX_R_INPUTS, tab_r_inputs);
	
  
	if (modbus_result < 0) return modbus_result;
  
	/* MAPPING */
	SV1 = tab_r_inputs[0];
	SV2 = tab_r_inputs[1];
  
  return 0;  
}



int mb_write_Q_R(void) {
  
  	/* Write holding registers */
	
	#ifdef MODBUS_DEBUG
	printf("Write Holding Registers\n");
	#endif
	
    
	/* MAPPRING */
	tab_r_outputs[0] = AZUIS;
	tab_r_outputs[1] = VERDES;
  
	if (mb == NULL)  mb_init();
	return modbus_write_registers(mb, 0, MAX_R_OUTPUTS, tab_r_outputs);
}