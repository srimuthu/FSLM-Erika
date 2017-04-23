#include <stdio.h>
#include <system.h>
#include <altera_avalon_pio_regs.h>
#include "sys/alt_irq.h"
#include "sys/alt_stdio.h"
#include "ee.h"
#include "fslm.h"
#include "ee_internal.h"
#include "sys/alt_alarm.h"

/*
7 sement LED - hex code equivalents

n - 0x6A
c - 0x72
- - 0x7E 

*/

int spin_lock=0;
int Task_spin=-1;
int Preemption_took_place=0;
const int EE_th_spin_prio[] = {0x1,0x1,0x1};
const int GlobalTaskID[] = {0,1,2};

static void handle_button_interrupts_cpu0(void* context, alt_u32 id){
    ActivateTask(task0);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_0_BASE, 0);
}

static void init_button_pio_0(){    
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(BUTTON_PIO_0_BASE, 0x1);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_0_BASE, 0x0);
    alt_irq_register( BUTTON_PIO_0_IRQ, NULL, handle_button_interrupts_cpu0 ); 
}

void printData2(void){
    alt_printf(" \n");
    alt_printf(" RQ0=%x %x %x %x", ResourceQ[0][0], ResourceQ[0][1], ResourceQ[0][2], ResourceQ[0][3] );
    alt_printf(" RQ1=%x %x %x %x", ResourceQ[1][0], ResourceQ[1][1], ResourceQ[1][2], ResourceQ[1][3] );
    alt_printf(" RQ2=%x %x %x %x", ResourceQ[2][0], ResourceQ[2][1], ResourceQ[2][2], ResourceQ[2][3] );
}

void printMeasurement(void){
    alt_printf(" \n");
    alt_printf(" CPU0=%x %x %x %x %x %x %x %x", MeasureQ[0][0], MeasureQ[0][1], MeasureQ[0][2], MeasureQ[0][3], MeasureQ[0][4], MeasureQ[0][5], MeasureQ[0][6], MeasureQ[0][7] );
    alt_printf(" CPU1=%x %x %x %x %x %x %x %x", MeasureQ[1][0], MeasureQ[1][1], MeasureQ[1][2], MeasureQ[1][3], MeasureQ[1][4], MeasureQ[1][5], MeasureQ[1][6], MeasureQ[1][7] );
    alt_printf(" CPU2=%x %x %x %x %x %x %x %x", MeasureQ[2][0], MeasureQ[2][1], MeasureQ[2][2], MeasureQ[2][3], MeasureQ[2][4], MeasureQ[2][5], MeasureQ[2][6], MeasureQ[2][7] );
    alt_printf(" CPU3=%x %x %x %x %x %x %x %x", MeasureQ[3][0], MeasureQ[3][1], MeasureQ[3][2], MeasureQ[3][3], MeasureQ[3][4], MeasureQ[3][5], MeasureQ[3][6], MeasureQ[3][7] );
}

void init_variables(void){
    int i=0, j=0;

    for(i=0; i<4; i++){
        EE_SHARED_DATA_var[i]=0;
        EE_SHARED_DATA_TailQ[i]=(int) &EE_SHARED_DATA_ResourceQ[i][0];
        for(j=0; j<10; j++){
            EE_SHARED_DATA_ResourceQ[j][i]=0xa0;
        }
    }
    
    for(i=0; i<NUM_CPU; i++){
        for(j=0; j<NUM_MEASUREMENT_PARAMS; j++){
            EE_SHARED_DATA_MeasureQ[i][j] = 0x00;
        }
    }
}

int main(){

  int k;
  alt_putstr("Ready!\n");
  StartOS(1);

  IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_0_BASE, 0xf);
  IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, 0x3F); // Display the character "-"
  alt_putstr("Set!\n");

    
  init_variables();
  init_button_pio_0();

  alt_putstr("Go!\n");
 
  while(1){
     for(k=0; k<5000000; k++){}
     printData2();
     printMeasurement();
  }
  return 0;
}

