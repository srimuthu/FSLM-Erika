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
const int EE_th_spin_prio[] = {0x4,0x4,0x4};
const int GlobalTaskID[] = {6,7,8};
EE_TID EE_resource_task[] = {-1, -1, -1};

volatile int timer_fired[] = {0,0,0};

#define TASK6_INTERVAL  1*alt_ticks_per_second()
#define TASK7_INTERVAL  2*alt_ticks_per_second()
#define TASK8_INTERVAL  3*alt_ticks_per_second()


alt_u32 Task6_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[0]++;
  ActivateTask(task6);
  return TASK6_INTERVAL;
}

alt_u32 Task7_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[1]++;
  ActivateTask(task7);
  return TASK7_INTERVAL;
}

alt_u32 Task8_alarm_callback (void* arg)
{
  /* Count the number of alarm expirations */
  timer_fired[2]++;
  ActivateTask(task8);
  return TASK8_INTERVAL;
}

int main(){

    alt_alarm task6Alarm;
    alt_alarm task7Alarm;
    alt_alarm task8Alarm;
    

    IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_2_BASE, 0x0);
    StartOS(1);
    IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_2_BASE, 0xf);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_2_BASE, 0x3F); // Display the character "-"

    /* start the periodic timers */
    alt_alarm_start (&task6Alarm, TASK6_INTERVAL, 
                   Task6_alarm_callback, NULL);
    alt_alarm_start (&task7Alarm, TASK7_INTERVAL, 
                   Task7_alarm_callback, NULL);
    alt_alarm_start (&task8Alarm, TASK8_INTERVAL, 
                   Task8_alarm_callback, NULL);

    while(1){;;}
    return 0;
}
