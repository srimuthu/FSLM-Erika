#include "ee_internal.h"
#include "ee_fslm_measure.h"

/* This function can be used to send a remote notification. Parameters: the remote notification. 
MUST BE >0 Returned values:  1 in case of error, 0 otherwise */

int EE_di_send(EE_TYPERN rn){
	

    register EE_UINT8 cpu;
    register EE_FREG flag;

    flag = EE_hal_begin_nested_primitive();
    
    /* here we suppose that inter-processor interrupts can not preempt this function. For that reason, it is not necessary to lock the
    spin lock here. That is, only one entity for each processor can use the notification data structure at a time, and the
    interprocessor interrupt always find the spin lock used by a task on another CPU or it find it free. */

    cpu = EE_rn_cpu[rn];
    
    if (cpu == EE_CURRENTCPU) {
        // THIS SHOULD NEVER HAPPEN Local notification not allowed      
        EE_hal_end_nested_primitive(flag);
        return 1;
    } else {       
        EE_hal_IRQ_interprocessor(cpu);  
    }  
    EE_hal_end_nested_primitive(flag); 
    
    return 0;
}
