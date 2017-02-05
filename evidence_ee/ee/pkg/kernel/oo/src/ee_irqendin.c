#include "ee_internal.h"
void EE_IRQ_end_instance(void){
	register EE_TID tmp;
	register EE_TID tmp_stacked;

	tmp = EE_rq_queryfirst();
	tmp_stacked = EE_stkfirst;

	if (tmp != EE_NIL && EE_sys_ceiling < EE_th_ready_prio[tmp]) {
		// we have to schedule a ready thread
		if (tmp_stacked != EE_NIL) {EE_th_status[tmp_stacked] = READY;}
		// and another task is put into the running state
		EE_th_status[tmp] = RUNNING;
		EE_sys_ceiling |= EE_th_dispatch_prio[tmp];
		EE_hal_IRQ_ready(EE_rq2stk_exchange());
	}else{EE_hal_IRQ_stacked(EE_stkfirst);}
}
