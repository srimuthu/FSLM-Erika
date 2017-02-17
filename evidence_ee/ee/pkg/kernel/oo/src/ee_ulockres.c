#include "ee_internal.h"

StatusType EE_oo_ReleaseResource(ResourceType ResID){
	EE_TID rq, current;
	EE_UREG isGlobal;
	register EE_FREG flag;
	extern int Preemption_took_place;

	isGlobal = ResID & EE_GLOBAL_MUTEX;
	ResID = ResID & ~EE_GLOBAL_MUTEX;

	if (ResID >= EE_MAX_RESOURCE) {	return E_OS_ID;}
	current = EE_stkfirst;
	if (EE_th_ready_prio[current] > EE_resource_ceiling[ResID]) {return E_OS_ACCESS;}
	
	flag = EE_hal_begin_nested_primitive();
        EE_resource_stack[ResID] = -1;
		EE_resource_locked[ResID] = 0;
		
		if (isGlobal) EE_hal_spin_out(ResID);
					

					

		rq = EE_rq_queryfirst();
		
		EE_sys_ceiling &=~0x80;
		EE_sys_ceiling &=~EE_th_spin_prio[EE_stkfirst];
		EE_sys_ceiling |= EE_th_dispatch_prio[EE_stkfirst];

		if(Preemption_took_place==1){
			Preemption_took_place=0;
			if(EE_th_ready_prio[EE_stkfirst]>= EE_th_ready_prio[rq] || rq == EE_NIL){							
				EE_hal_stkchange(EE_stkfirst);
			}
		}

		
		
		if (rq != EE_NIL) {
			if (EE_sys_ceiling < EE_th_ready_prio[rq]) {
				EE_th_status[current] = READY;
				EE_th_status[rq] = RUNNING;
				EE_sys_ceiling |= EE_th_dispatch_prio[rq];
	
	
				EE_hal_ready2stacked(EE_rq2stk_exchange());
			}
		}
		
		
		 
	EE_hal_end_nested_primitive(flag); 

	return E_OK;
}
