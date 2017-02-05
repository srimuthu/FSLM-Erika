#include "ee_internal.h"

/* This function can be used to send a remote notification. Parameters: the remote notification. 
MUST BE >0 Returned values:  1 in case of error, 0 otherwise */

int EE_rn_send(EE_TYPERN rn, EE_TYPERN t, EE_TYPERN_PARAM par){
	register EE_UINT8 cpu;
	register EE_TYPERN_SWITCH sw;
	register int newIRQ;
	register EE_FREG flag;
	
	flag = EE_hal_begin_nested_primitive();
	
	/* here we suppose that inter-processor interrupts can not preempt this function. For that reason, it is not necessary to lock the
	spin lock here. That is, only one entity for each processor can use the notification data structure at a time, and the
	interprocessor interrupt always find the spin lock used by a task on another CPU or it find it free. */

	cpu = EE_rn_cpu[rn];
	ResourceQ[1][EE_CURRENTCPU]=cpu;
	
	if (cpu == EE_CURRENTCPU) {
		// THIS SHOULD NEVER HAPPEN Local notification not allowed      
		EE_hal_end_nested_primitive(flag);
		return 1;
	} else {
		EE_hal_spin_in_int(EE_rn_spin[cpu]);// Spin Lock acquisition 
		
			/* Note: sw must be read inside the spin-lock because its value can be changed by the other CPU! */
			sw = EE_rn_switch[cpu] & EE_RN_SWITCH_COPY;

			/* Check if we should raise a new Interprocessor Interrupt. That is, none is inside the IIRQ interrupt handler, and noone
			 * is already queued on the current data structure */
			newIRQ = !(EE_rn_switch[cpu] & EE_RN_SWITCH_INSIDEIRQ) && EE_rn_first[cpu][sw] == -1;

			/* the interrupt handler have to do the cycle again */
			if (EE_rn_switch[cpu] & EE_RN_SWITCH_INSIDEIRQ){ EE_rn_switch[cpu] |= EE_RN_SWITCH_NEWRN;}

			if (!EE_rn_type[rn][sw]) {// Queuing request 
				/* request was not queued before, insert it into the pending requests */
				EE_rn_next[rn][sw] = EE_rn_first[cpu][sw];
				EE_rn_first[cpu][sw] = rn;
			}
			EE_rn_pending[rn][sw] += par.pending;// increase the pending counter 
			EE_rn_type[rn][sw] |= t;// set the type in the remote notification 

		EE_hal_spin_out_int(EE_rn_spin[cpu]);// Spin Lock release
			
		/* Inter-processor interrupt. We raise an interprocessor interrupt only if there is not a
		similar interrupt pending. Note that the irq is raised before releasing the spin lock */
		EE_hal_IRQ_interprocessor(cpu);  
	}  
	EE_hal_end_nested_primitive(flag);
	
	
	return 0;
}
