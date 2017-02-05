#include "ee.h"
#include "ee_internal.h"



void EE_rn_execute(EE_TYPERN rn, EE_UINT8 sw){
						
	
	
	
	if (EE_rn_type[rn][sw] & 0x40){
		extern int spin_lock;
		extern int Preemption_took_place;
		
		
		
		EE_sys_ceiling|=0x80;
		spin_lock=0;
		if(EE_rn_task[rn]!=EE_stkfirst){
			ResourceQ[2][2]++;
			Preemption_took_place=1;
			
				PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
				PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
				PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,1);

			EE_hal_stkchange(rn);			
		}
		
		EE_rn_type[rn][sw] &= ~0x40;
		
		
		
	}
}



void EE_rn_handler(void){
  register EE_TYPERN current;
  register EE_TYPERN_SWITCH sw;
  int redo = 0;
  			
  
  do {
    sw = EE_rn_switch[EE_CURRENTCPU] & EE_RN_SWITCH_COPY;
	
	ResourceQ[1][EE_CURRENTCPU]= EE_rn_first[EE_CURRENTCPU][sw];
	
	EE_hal_IRQ_begin_primitive();
 	    EE_hal_spin_in_int(EE_rn_spin[EE_CURRENTCPU]);// Spin Lock acquisition 
			// switch pending requests and set the inside irq flag 
			EE_rn_switch[EE_CURRENTCPU] = (EE_rn_switch[EE_CURRENTCPU] ^ EE_RN_SWITCH_COPY) |	EE_RN_SWITCH_INSIDEIRQ;
		EE_hal_spin_out_int(EE_rn_spin[EE_CURRENTCPU]);// Spin Lock release
	EE_hal_IRQ_end_primitive();
       
	   
	  
	for (current = EE_rn_first[EE_CURRENTCPU][sw]; current != -1; current = EE_rn_next[current][sw]) {
		EE_rn_execute(current,sw);

	}
    EE_rn_first[EE_CURRENTCPU][sw] = -1;
	
					
                    

	EE_hal_IRQ_begin_primitive();
		EE_hal_spin_in_int(EE_rn_spin[EE_CURRENTCPU]);
			/* if the other processor has queued another request */
			if (EE_rn_switch[EE_CURRENTCPU] & EE_RN_SWITCH_NEWRN) {
				EE_rn_switch[EE_CURRENTCPU] &= ~EE_RN_SWITCH_NEWRN;// reset the newrn flag 
				redo = 1;// redo the dispatching of the remote notifications
			}else {
				redo = 0;// we can exit the interrupt! 
				EE_rn_switch[EE_CURRENTCPU] &= ~EE_RN_SWITCH_INSIDEIRQ;// set that we are no more inside the interrupt
				EE_hal_IRQ_interprocessor_served(EE_CURRENTCPU);
			}
		EE_hal_spin_out_int(EE_rn_spin[EE_CURRENTCPU]);// Spin Lock release 
	if (redo){EE_hal_IRQ_end_primitive();}// note: we end the handler with interrupt disabled!

  
  } while (redo);
}
