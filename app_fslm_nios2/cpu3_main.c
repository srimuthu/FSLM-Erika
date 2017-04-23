#include <stdio.h>
#include <system.h>
#include <altera_avalon_pio_regs.h>
#include "sys/alt_irq.h"
#include "ee.h"
#include "fslm.h"
#include "ee_internal.h"
#include "sys/alt_alarm.h"

int spin_lock=0;
int Task_spin=-1;
int Preemption_took_place=0;
const int EE_th_spin_prio[] = {0x2,0x2,0x2,0x2};
const int GlobalTaskID[] = {9,10,11,12};
volatile int timer_fired[] = {0,0,0,0};

#define TASK9_INTERVAL  1*alt_ticks_per_second()
#define TASK10_INTERVAL  2*alt_ticks_per_second()
#define TASK11_INTERVAL  3*alt_ticks_per_second()
#define TASK12_INTERVAL  4*alt_ticks_per_second()


alt_u32 Task9_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[0]++;
  ActivateTask(task9);
  return TASK9_INTERVAL;
}

alt_u32 Task10_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[1]++;
  ActivateTask(task10);
  return TASK10_INTERVAL;
}

alt_u32 Task11_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[2]++;
  ActivateTask(task11);
  return TASK11_INTERVAL;
}

alt_u32 Task12_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[3]++;
  ActivateTask(task12);
  return TASK12_INTERVAL;
}

int main(){
    
    IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_3_BASE, 0x0);
    StartOS(1);
    IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_3_BASE, 0xf);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_3_BASE, 0x3F); // Display the character "-"

    /* start the periodic timers */
    alt_alarm_start (&task9Alarm, TASK9_INTERVAL, 
                   Task9_alarm_callback, NULL);
    alt_alarm_start (&task10Alarm, TASK10_INTERVAL, 
                   Task10_alarm_callback, NULL);
    alt_alarm_start (&task11Alarm, TASK11_INTERVAL, 
                   Task11_alarm_callback, NULL);
    alt_alarm_start (&task12Alarm, TASK12_INTERVAL, 
                   Task12_alarm_callback, NULL);

    while(1){;;}
    return 0;
}
