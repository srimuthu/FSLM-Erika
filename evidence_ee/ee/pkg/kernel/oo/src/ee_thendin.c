#include "ee_internal.h"

void EE_thread_end_instance(void){
    EE_TID current, rqfirst;
    EE_TID TaskID;
    current = EE_stkfirst;

    // increase the remaining activations...
    EE_th_rnact[current]++;

    /* The task state switch from STACKED TO READY because it end its
     * instance. Note that status=READY and nact=0 ==>> the task is
     * suspended!!! */
    EE_th_status[current] = SUSPENDED;

    // reset the thread priority bit in the system_ceiling
    EE_sys_ceiling &= ~EE_th_dispatch_prio[current];

    // extract the task from the stk data structure 
     EE_stkfirst = EE_th_next[EE_stkfirst];

    // if we called a ChainTask, EE_th_terminate_nextask[current] != NIL
    TaskID = EE_th_terminate_nextask[current];
    if (TaskID != EE_NIL) {
		/* see also activate.c
		put the task in the ready state:
		- if the task is basic/BCC1 or extended it can be here only because
		it had rnact=1 before the call, and so it is in suspended state
		- if the task is basic/BCC2 it can be that it is ready or 
		running. in that case we have to check and queue it anyway */
		EE_th_status[TaskID] = READY;
		/* insert the task in the ready queue */
		EE_rq_insert(TaskID);
    }


    /* check if there is to schedule a ready thread or pop a preempted thread */
    rqfirst = EE_rq_queryfirst();
    if (rqfirst == EE_NIL) {
		/* No threads in the ready queue, return to the preempted thread (maybe main) */	
		if (EE_stkfirst != EE_NIL) {EE_th_status[EE_stkfirst] = RUNNING;}
		EE_hal_endcycle_stacked(EE_stkfirst);
    }else if (EE_sys_ceiling >= EE_th_ready_prio[rqfirst]) {
		// we have to schedule an interrupted thread (already on the stack!!!)
		EE_th_status[EE_stkfirst] = RUNNING;
		EE_hal_endcycle_stacked(EE_stkfirst);
    }else { 
		// we have to schedule a ready thread
		EE_th_status[rqfirst] = RUNNING;
		EE_sys_ceiling |= EE_th_dispatch_prio[rqfirst];
		EE_hal_endcycle_ready(EE_rq2stk_exchange());
    }
    /* Remember: after hal_endcycle_XXX there MUST be NOTHING!!! */
}  
