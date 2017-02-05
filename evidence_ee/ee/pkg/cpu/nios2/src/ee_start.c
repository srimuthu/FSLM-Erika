#include "ee_internal.h"
#include "kernel/oo/inc/ee_common.h"
#include "sys/alt_stdio.h"
static void EE_nios2_IIRQ_handler(void *arg, alt_u32 intno){
  alt_u32 pending;
  pending = alt_irq_interruptible(EE_IPIC_IRQ);  
  EE_rn_handler();
  alt_irq_non_interruptible(pending);
}

int EE_cpu_startos(void){
	alt_irq_register( EE_IPIC_IRQ, 0, EE_nios2_IIRQ_handler );// Register the Interprocessor IRQ handler
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK( EE_IPIC_INPUT_BASE, 1 );// enable interrupts for the local IPIC
	if (EE_has_startup_barrier) {
		if (EE_CURRENTCPU){
			EE_altera_mutex_spin_in();
			EE_startsynclocation--;
			EE_altera_mutex_spin_out();
		}else{// CPU0, when it starts the mutex is already locked
			EE_startsynclocation = EE_MAX_CPU-1;
			EE_altera_mutex_spin_out();
		}
		while (EE_startsynclocation){
#if EE_CURRENTCPU==0
//	alt_printf("%x\n", EE_startsynclocation);


#endif		
		;// wait for all the processors to start
		}
	}
	return 0;
}
