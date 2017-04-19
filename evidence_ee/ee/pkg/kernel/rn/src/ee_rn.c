#include "ee.h"
#include "ee_internal.h"
#include "ee_fslm_measure.h"

void EE_rn_execute(ResourceType ResID, EE_TID requesting_task){

	
	if(ResID != 0xff && requesting_task != -1){
		extern int spin_lock;
        extern int Preemption_took_place;
        EE_sys_ceiling |= 0x80; 
		spin_lock = 0;
	
		if( requesting_task != EE_stkfirst){
			Preemption_took_place = 1;
			EE_hal_stkchange(requesting_task);
		}
		
	}

}

void EE_rn_handler(void){
#ifdef MF_INTR_HANDLER
    PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
    PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,0);
#endif

		int i;
        ResourceType ResID = 0xff;
        EE_TID requesting_task = -1;

        EE_hal_IRQ_begin_primitive();
        // Find which resource is locked
        for(i=0; i< EE_MAX_RESOURCE ; i++){
            if(EE_resource_locked[i]){
                ResID = i;
                requesting_task = EE_resource_stack[ResID];               
            }
        }
		EE_hal_IRQ_end_primitive();
		
	#ifdef MF_INTR_HANDLER	
		PERF_END(PERFORMANCE_COUNTER_0_BASE,0);
		PERF_STOP_MEASURING(PERFORMANCE_COUNTER_0_BASE);
		MeasureQ[EE_CURRENTCPU][MF_INTR_HANDLER] = perf_get_section_time((void *)PERFORMANCE_COUNTER_0_BASE, 0);
	#endif
	#ifdef MF_INTR_EXEC
		PERF_RESET(PERFORMANCE_COUNTER_1_BASE);
		PERF_START_MEASURING(PERFORMANCE_COUNTER_1_BASE);
		PERF_BEGIN(PERFORMANCE_COUNTER_1_BASE,0);
	#endif
	
		EE_rn_execute(ResID, requesting_task);

        EE_hal_IRQ_begin_primitive();
        EE_hal_IRQ_interprocessor_served(EE_CURRENTCPU);
	
	#ifdef MF_INTR_EXEC
		PERF_END(PERFORMANCE_COUNTER_1_BASE,0);
		PERF_STOP_MEASURING(PERFORMANCE_COUNTER_1_BASE);
		MeasureQ[EE_CURRENTCPU][MF_INTR_EXEC] = perf_get_section_time((void *)PERFORMANCE_COUNTER_1_BASE, 0);
	#endif	

	

}
