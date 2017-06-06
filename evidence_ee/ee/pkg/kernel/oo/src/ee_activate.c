#include "ee_internal.h"

StatusType EE_oo_ActivateTask(TaskType TaskID){
	register TaskType tmp,current;
	register EE_FREG flag;

	//activate a task on another core
	if (TaskID & EE_REMOTE_TID) {
		EE_TYPERN_PARAM par;
		par.pending = 1;
		//EE_rn_send(TaskID & ~EE_REMOTE_TID, EE_RN_TASK, par );
		return E_OK;
	}

	//error check
	if (TaskID < 0 || TaskID >= EE_MAX_TASK) { return E_OS_ID;}
	flag = EE_hal_begin_nested_primitive();
		//error check
		if (EE_th_rnact[TaskID] == 0) {
			EE_hal_end_nested_primitive(flag);
			return E_OS_LIMIT;
		} 
		EE_th_rnact[TaskID]--;

		if (EE_th_status[TaskID] == SUSPENDED) {EE_th_status[TaskID] = READY;}
		EE_rq_insert(TaskID);

		if (!EE_hal_get_IRQ_nesting_level()) {
			//return the task at the head of the ready queue
			tmp = EE_rq_queryfirst();
			if (tmp != EE_NIL) {
				if (EE_sys_ceiling < EE_th_ready_prio[tmp]) {
					current = EE_stkfirst;
					if (current != EE_NIL) {EE_th_status[current] = READY;}
					EE_th_status[tmp] = RUNNING;
					EE_sys_ceiling |= EE_th_dispatch_prio[tmp];
					//put the active task on the stack
					//let the PC program counter point to the task at the head of the queue
					EE_hal_ready2stacked(EE_rq2stk_exchange());
				}
			}
		}
	EE_hal_end_nested_primitive(flag);
	return E_OK;
}
