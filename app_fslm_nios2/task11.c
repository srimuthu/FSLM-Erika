#include <stdio.h>
#include <system.h>
#include "ee.h"
#include "sys/alt_irq.h"
#include <altera_avalon_pio_regs.h>
#include "ee_internal.h"
#include "sys/alt_stdio.h"

/*
task11 - No resource access
*/

TASK(task11){   
             
    
    int j=0, i=0;
        for(j=0; j<3; j++){
            IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_3_BASE, 0xb);
            for(i=0; i<500000; i++){}
            IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_3_BASE, 0x0);
            for(i=0; i<500000; i++){}
        }
        IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_3_BASE, 0x0);
    TerminateTask();
}
