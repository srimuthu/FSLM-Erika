/* Altera Includes */ 
#include "system.h"
#include <altera_avalon_pio_regs.h>
#include <stdio.h>
#include "sys/alt_alarm.h"

/* EE includes */
#include "ee.h"

/* a constant used in the delay loops */
#define ONEMILLION 1000000

/* this counter will be printed on the console in ascending 
 * or decreasing order when the button is pressed
 */
volatile int mycounter=0;

/* This alarm callback is attached to the system timer, and is used to
 * activate LowTask
 * The period in expressed in system timer ticks, each one typically 10ms 
 */
#define LOWTASK_TIMER_INTERVAL  400
alt_u32 LowTask_alarm_callback (void* arg)
{
  ActivateTask(LowTask);
  return LOWTASK_TIMER_INTERVAL;
}

/*
 * This is the Low priority task.
 * When the task runs, it locks a resource for a -long- time, preventing 
 * HighTask to preempt.
 */

TASK(LowTask)
{
  int i,j;

  /* Lock the resource */
  GetResource(Resource);

  /* do a long loop, printing the numbers from 0 to 9 */
  IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, 0x01);
  for (j=0; j<10; j++) {
    for (i=0; i<ONEMILLION; i++);
    printf("%d", j);
  }
  printf("\n");
  IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, 0x00);
  
  /* Release the lock */
  ReleaseResource(Resource);
  
  TerminateTask();
}


/* High Priority Task.
 * This task simply decrements the counter depending on the current 
 * applicationmode, and prints it using printf.
 */
TASK(HighTask)
{
  int i;
  AppModeType currentmode;
  
  /* get the current application mode */
  currentmode = GetActiveApplicationMode();
  
  GetResource(Resource);

  IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, 0x02);
  for (i=0; i<ONEMILLION; i++);
  
  /* decrement or increment the counter depending on the application mode */
  if ( currentmode==ModeIncrement )
    mycounter++;
  else
    mycounter--;
  printf("mycounter=%d\n", mycounter);
  IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, 0x00);

  ReleaseResource(Resource);

  TerminateTask();
}

/*
 * Handle button_pio interrupts activates HighTask.
 */
static void handle_button_interrupts(void* context, alt_u32 id)
{
  /* Reset the Button's edge capture register. */
  IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0);

  ActivateTask(HighTask);  
}


/* Initialize the button_pio. */
static void init_button_pio()
{
  /* Enable the first button interrupt. */
  IOWR_ALTERA_AVALON_PIO_IRQ_MASK(BUTTON_PIO_BASE, 0x1);
  /* Reset the edge capture register. */
  IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0x0);
  /* Register the interrupt handler. */
  alt_irq_register( BUTTON_PIO_IRQ, NULL, handle_button_interrupts ); 
}


int main(void)
{
  AppModeType startupmode;
  alt_alarm myalarm;
  
  int buttonstatus;
  
  printf("Welcome to the ERIKA Enterprise Resource demo!\n\n");
  
  /* set the stack space to a known pattern, to allow stack statistics by
   * Lauterbach Trace32 */
  //EE_trace32_stack_init();
  
  /* read the button status */
  buttonstatus = IORD_ALTERA_AVALON_PIO_DATA(BUTTON_PIO_BASE);

  /* check if the first button is pressed or not */
  if (buttonstatus & 0x1) {
    /* the button is not pressed */
    startupmode = ModeIncrement;
    printf("ModeIncrement selected.\n");
    printf("To select ModeDecrement, start the demo with the 1st button pressed!\n");
  }
  else {
    /* the button is pressed */
    startupmode = ModeDecrement;
    mycounter = 1000;
    printf("ModeDecrement selected.\n");
    printf("To select ModeIncrement, start the demo with the 1st button not pressed!\n");
  }

  /* program the Button PIO interrupt*/
  init_button_pio();
  
  /* start the periodic timers */
  alt_alarm_start (&myalarm, LOWTASK_TIMER_INTERVAL, 
                   LowTask_alarm_callback, NULL);

  StartOS(startupmode);
    
  for (;;);
}
