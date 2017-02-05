#include "ee_internal.h"

StatusType EE_oo_TerminateTask(void){
	EE_hal_begin_primitive();
	EE_th_terminate_nextask[EE_stkfirst] = EE_NIL;
	EE_hal_terminate_task(EE_stkfirst);
	// This return instruction usually is optimized by the compiler, because hal_terminate_task does not return...
	return E_OK;
}

