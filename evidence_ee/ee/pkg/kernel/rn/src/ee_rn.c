#include "ee.h"
#include "ee_internal.h"

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

		EE_rn_execute(ResID, requesting_task);
		

        EE_hal_IRQ_begin_primitive();
        EE_hal_IRQ_interprocessor_served(EE_CURRENTCPU);
}
