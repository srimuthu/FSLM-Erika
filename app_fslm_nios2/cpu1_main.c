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
const int EE_th_spin_prio[] = {0x2,0x2,0x2};
const int GlobalTaskID[] = {3,4,5};
EE_TID EE_resource_task[] = {-1, -1, -1};

volatile int timer_fired[] = {0,0,0,0,0};


#define TASK1_INTERVAL  alt_ticks_per_second()
#define TASK2_INTERVAL  2*alt_ticks_per_second()
#define TASK3_INTERVAL  3*alt_ticks_per_second()
#define TASK4_INTERVAL  4*alt_ticks_per_second()
#define TASK5_INTERVAL  5*alt_ticks_per_second()


alt_u32 Task1_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[0]++;
  ActivateTask(task1);
  return TASK1_INTERVAL;
}

alt_u32 Task2_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[1]++;
  ActivateTask(task2);
  return TASK2_INTERVAL;
}

alt_u32 Task3_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[2]++;
  ActivateTask(task3);
  return TASK3_INTERVAL;
}

alt_u32 Task4_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[3]++;
  ActivateTask(task4);
  return TASK4_INTERVAL;
}

alt_u32 Task5_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[4]++;
  ActivateTask(task5);
  return TASK5_INTERVAL;
}

int main(){
    
    alt_alarm task1Alarm;
    alt_alarm task2Alarm;
    alt_alarm task3Alarm;
    alt_alarm task4Alarm;
    alt_alarm task5Alarm;


    IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_1_BASE, 0x0);
    StartOS(1);
    IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_1_BASE, 0xf);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, 0x3F); // Display the character "-"

    /* start the periodic timers */
    alt_alarm_start (&task1Alarm, TASK1_INTERVAL, 
                   Task1_alarm_callback, NULL);
    alt_alarm_start (&task2Alarm, TASK2_INTERVAL, 
                   Task2_alarm_callback, NULL);
    alt_alarm_start (&task3Alarm, TASK3_INTERVAL, 
                   Task3_alarm_callback, NULL);
    alt_alarm_start (&task4Alarm, TASK4_INTERVAL, 
                   Task4_alarm_callback, NULL);
    alt_alarm_start (&task5Alarm, TASK5_INTERVAL, 
                   Task5_alarm_callback, NULL);

    while(1){;;}      
    return 0;
}
