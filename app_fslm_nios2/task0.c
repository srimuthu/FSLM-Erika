#include <stdio.h>
#include <system.h>
#include "ee.h"
#include "sys/alt_irq.h"
#include "ee_internal.h"
#include "sys/alt_stdio.h"

/*
Task for clearing the measurement queue
Triggered by button-0 on the master CPU - cpu0
Use manually when resetting the values in MeasureQ
*/

TASK(task0){   
             
    
    int j=0, i=0;

    for(i=0; i<NUM_CPU; i++){
        for(j=0; j<NUM_MEASUREMENT_PARAMS; j++){
            MeasureQ[i][j] = 0x00;
        }
    }

    TerminateTask();
}
