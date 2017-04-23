#include <stdio.h>
#include <system.h>
#include "ee.h"
#include "shareddata.h"
#include "resource_0_data.h"
#include "sys/alt_irq.h"
#include <altera_avalon_pio_regs.h>
#include "ee_internal.h"
#include "sys/alt_stdio.h"

/*
Task 9 - Task utilizing global resource mutex0
*/

TASK(task9){   
             
    int j=0, i=0;

    // Non-critical section
        for(j=0; j<2; j++){
            IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_3_BASE, 0x9);
            for(i=0; i<50000; i++){}
            IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_3_BASE, 0x0);
            for(i=0; i<50000; i++){}
        }

    // Critical section
    GetResource(mutex0);
        mutex0_mydata = 9;
    ReleaseResource(mutex0);

    // Non-critical section
        for(j=0; j<2; j++){
            IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_3_BASE, 0x9);
            for(i=0; i<50000; i++){}
            IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_3_BASE, 0x0);
            for(i=0; i<50000; i++){}
        }

        IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_3_BASE, 0x0);
    TerminateTask();
}


