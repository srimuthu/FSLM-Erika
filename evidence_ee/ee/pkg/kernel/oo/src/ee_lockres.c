#include "ee_internal.h"
#include "ee_fslm_measure.h"

StatusType EE_oo_GetResource(ResourceType ResID){

#ifdef MF_MUTEX
	int dummy = 0;
	int iter = 0;
	
	if(EE_CURRENTCPU == 1){
		PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
		PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
	}
	else if (EE_CURRENTCPU == 2){
		PERF_RESET(PERFORMANCE_COUNTER_1_BASE);
		PERF_START_MEASURING(PERFORMANCE_COUNTER_1_BASE);
	}
	else {
		PERF_RESET(PERFORMANCE_COUNTER_2_BASE);
		PERF_START_MEASURING(PERFORMANCE_COUNTER_2_BASE);
	}	
		
	for(iter=0;iter<100000;iter++){
		if(EE_CURRENTCPU == 1){
			PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,0);
		}
		else if (EE_CURRENTCPU == 2){
			PERF_BEGIN(PERFORMANCE_COUNTER_1_BASE,0);
		}
		else{
			PERF_BEGIN(PERFORMANCE_COUNTER_2_BASE,0);
		}
			EE_altera_mutex_spin_in();
			dummy++;
			EE_altera_mutex_spin_out();
		if(EE_CURRENTCPU == 1){
			PERF_END(PERFORMANCE_COUNTER_0_BASE,0);
		}
		else if (EE_CURRENTCPU == 2){
			PERF_END(PERFORMANCE_COUNTER_1_BASE,0);
		}
		else{
			PERF_END(PERFORMANCE_COUNTER_2_BASE,0);
		}
	}
	
	if(EE_CURRENTCPU == 1){
		PERF_STOP_MEASURING(PERFORMANCE_COUNTER_0_BASE);
		MeasureQ[EE_CURRENTCPU][0] = perf_get_section_time((void *)PERFORMANCE_COUNTER_0_BASE, 0);
		MeasureQ[EE_CURRENTCPU][1] = perf_get_num_starts((void *)PERFORMANCE_COUNTER_0_BASE, 0);
		MeasureQ[EE_CURRENTCPU][2] = dummy;
	}
	else if(EE_CURRENTCPU == 2){
		PERF_STOP_MEASURING(PERFORMANCE_COUNTER_1_BASE);
		MeasureQ[EE_CURRENTCPU][0] = perf_get_section_time((void *)PERFORMANCE_COUNTER_1_BASE, 0);
		MeasureQ[EE_CURRENTCPU][1] = perf_get_num_starts((void *)PERFORMANCE_COUNTER_1_BASE, 0);
		MeasureQ[EE_CURRENTCPU][2] = dummy;
	}
	else{
		PERF_STOP_MEASURING(PERFORMANCE_COUNTER_2_BASE);
		MeasureQ[EE_CURRENTCPU][0] = perf_get_section_time((void *)PERFORMANCE_COUNTER_2_BASE, 0);
		MeasureQ[EE_CURRENTCPU][1] = perf_get_num_starts((void *)PERFORMANCE_COUNTER_2_BASE, 0);
		MeasureQ[EE_CURRENTCPU][2] = dummy;
	}
	
#endif MF_MUTEX
	
#ifdef MF_REQ_ADMIN
    PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
    PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,0);
#endif
	
	register EE_UREG isGlobal;
	register EE_FREG flag;

	isGlobal = ResID & EE_GLOBAL_MUTEX;
	ResID = ResID & ~EE_GLOBAL_MUTEX;
	if (ResID >= EE_MAX_RESOURCE) { return E_OS_ID; }
	if (EE_resource_locked[ResID] || EE_th_ready_prio[EE_stkfirst] > EE_resource_ceiling[ResID]) {return E_OS_ACCESS;}

	flag = EE_hal_begin_nested_primitive();
        EE_resource_task[0] = ResID;
		EE_resource_task[1] = EE_stkfirst;
		EE_resource_locked[ResID] = 1;
		EE_sys_ceiling |= 0x80;


		
		if (isGlobal){ EE_hal_spin_in(ResID);}
	EE_hal_end_nested_primitive(flag);   	

	return E_OK;
}
