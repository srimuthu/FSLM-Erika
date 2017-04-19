#include "ee_internal.h"
#include "ee_fslm_measure.h"

/* This function can be used to send a remote notification. Parameters: the remote notification. 
MUST BE >0 Returned values:  1 in case of error, 0 otherwise */

int EE_rn_send(EE_TYPERN rn, EE_TYPERN t, EE_TYPERN_PARAM par){
	
#ifdef MF_INTR_SEND
    PERF_RESET(PERFORMANCE_COUNTER_1_BASE);
    PERF_START_MEASURING(PERFORMANCE_COUNTER_1_BASE);
    PERF_BEGIN(PERFORMANCE_COUNTER_1_BASE,0);
#endif
	
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

#ifdef MF_INTR_SEND	
	PERF_END(PERFORMANCE_COUNTER_1_BASE,0);
	PERF_STOP_MEASURING(PERFORMANCE_COUNTER_1_BASE);
    MeasureQ[EE_CURRENTCPU][MF_INTR_SEND] = perf_get_section_time((void *)PERFORMANCE_COUNTER_1_BASE, 0);
#endif    
    
    return 0;
}
