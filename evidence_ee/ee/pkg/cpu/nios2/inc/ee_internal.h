#include "cpu/nios2/inc/ee_cpu.h"
#include <altera_avalon_mutex.h>
#include <altera_avalon_mutex_regs.h>
#include <altera_avalon_pio_regs.h>

#ifndef __INCLUDE_NIOS2_INTERNAL_H__
#define __INCLUDE_NIOS2_INTERNAL_H__
#define EE_hal_IRQ_stacked EE_hal_endcycle_stacked
#define EE_hal_IRQ_ready EE_hal_endcycle_ready
#define __OO_CPU_HAS_STARTOS_ROUTINE__

#include <altera_avalon_mutex.h>
#include <altera_avalon_mutex_regs.h>
#include <altera_avalon_pio_regs.h>

#include "ee_internal.h"
#include "SPIN_PRIO.h"

#include "sys/alt_stdio.h"

#include "altera_avalon_performance_counter.h"

extern EE_UREG EE_hal_endcycle_next_thread;
extern EE_UREG EE_hal_endcycle_next_tos;
extern EE_UREG EE_IRQ_nesting_level;
extern volatile EE_UREG EE_startsynclocation EE_DISABLE_GP_ADDRESSING;
extern const EE_UREG EE_has_startup_barrier EE_DISABLE_GP_ADDRESSING;
extern alt_u32 *const EE_IPIC_OUTPUT_BASE;
extern alt_u32 *const EE_IPIC_INPUT_BASE;
extern alt_u32 const EE_IPIC_IRQ;


#ifndef FSLM2_H_
#define FSLM2_H_
#include "C:\altera\91\nios2eds\components\evidence_ee\ee\pkg\kernel\rn\inc\ee_rn_internal.h"

extern EE_UINT32 EE_SHARED_DATA_var[4];
extern EE_UINT32 * Pvar;
extern EE_UINT32 var[4];

extern EE_UINT32 EE_SHARED_DATA_TailQ[10];
extern EE_UINT32 * PTailQ;
extern EE_UINT32 TailQ[10];

extern EE_UINT32 * ResourceQ[10];
extern EE_UINT32 RQ_cpu0[10];
extern EE_UINT32 RQ_cpu1[10];
extern EE_UINT32 RQ_cpu2[10];
extern EE_UINT32 RQ_cpu3[10];
extern EE_UINT32 RQ_cpu4[10];
extern EE_UINT32 RQ_cpu5[10];
extern EE_UINT32 RQ_cpu6[10];
extern EE_UINT32 RQ_cpu7[10];
extern EE_UINT32 RQ_cpu8[10];
extern EE_UINT32 RQ_cpu9[10];


extern int spin_lock;
extern const int EE_th_spin_prio[];
extern const int GlobalTaskID[];

  


#endif /*FSLM2_H_*/

__INLINE__ void __ALWAYS_INLINE__ EE_hal_begin_primitive(void){EE_hal_disableIRQ();}
__INLINE__ void __ALWAYS_INLINE__ EE_hal_IRQ_begin_primitive(void){EE_hal_disableIRQ();}
__INLINE__ void __ALWAYS_INLINE__ EE_hal_end_primitive(void){EE_hal_enableIRQ();}
__INLINE__ void __ALWAYS_INLINE__ EE_hal_IRQ_end_primitive(void){EE_hal_enableIRQ();}
__INLINE__ void __ALWAYS_INLINE__ EE_hal_end_nested_primitive(EE_FREG f){NIOS2_WRITE_STATUS (f);}
__INLINE__ EE_UREG __ALWAYS_INLINE__ EE_hal_get_IRQ_nesting_level(void){return EE_IRQ_nesting_level;}

int EE_cpu_startos(void);
void EE_nios2_terminate_savestk(EE_ADDR sp, EE_ADDR realbody);
void EE_nios2_terminate_task(EE_ADDR sp) NORETURN;

/* Same as alt_irq_disable_all in altera_nios2/HAL/inc/sys/alt_irq.h */
__INLINE__ EE_FREG __ALWAYS_INLINE__ EE_hal_begin_nested_primitive(void){
  EE_FREG context;
  NIOS2_READ_STATUS (context);
  NIOS2_WRITE_STATUS (0);
  return context;
}

void EE_nios2_hal_ready2stacked(EE_ADDR thread_addr, EE_UREG tos_index); /* in ASM */
__INLINE__ void __ALWAYS_INLINE__ EE_hal_ready2stacked(EE_TID thread){
    EE_nios2_hal_ready2stacked(EE_hal_thread_body[thread],EE_hal_thread_tos[thread+1]);
}

/* typically called at the end of a thread instance */
__INLINE__ void __ALWAYS_INLINE__ EE_hal_endcycle_stacked(EE_TID thread){
  EE_hal_endcycle_next_tos = EE_hal_thread_tos[thread+1];
  EE_hal_endcycle_next_thread = 0;
}

__INLINE__ void __ALWAYS_INLINE__ EE_hal_endcycle_ready(EE_TID thread){
  EE_hal_endcycle_next_tos = EE_hal_thread_tos[thread+1];
  EE_hal_endcycle_next_thread = (EE_UREG)EE_hal_thread_body[thread];
}

/* called to change the active stack, typically inside blocking primitives */
/* there is no mono version for this primitive...*/
void EE_nios2_hal_stkchange(EE_UREG, EE_UREG tos_index); /* in ASM */
__INLINE__ void __ALWAYS_INLINE__ EE_hal_stkchange(EE_TID thread){
    EE_nios2_hal_stkchange(0, EE_hal_thread_tos[thread+1]);
}

__INLINE__ void __ALWAYS_INLINE__ EE_hal_terminate_savestk(EE_TID t){
  EE_nios2_terminate_savestk(&EE_terminate_data[t],
				(EE_ADDR)EE_terminate_real_th_body[t]);
}

__INLINE__ void __ALWAYS_INLINE__ EE_hal_terminate_task(EE_TID t){
  EE_nios2_terminate_task(&EE_terminate_data[t]);
}

extern EE_UINT32 EE_hal_spin_status[]; 
extern EE_UINT32 * const EE_hal_spin_value[];


__INLINE__ void __ALWAYS_INLINE__ EE_altera_mutex_spin_in(void){
	alt_u32 data, check;
	/* the data we want the mutex to hold */
	data = (EE_CURRENTCPU << ALTERA_AVALON_MUTEX_MUTEX_OWNER_OFST) | 1;

	do {
		/* attempt to write to the mutex */
		IOWR_ALTERA_AVALON_MUTEX_MUTEX(EE_ALTERA_MUTEX_BASE, data);
		check = IORD_ALTERA_AVALON_MUTEX_MUTEX(EE_ALTERA_MUTEX_BASE);
	} while ( check != data);
}

__INLINE__ void __ALWAYS_INLINE__ EE_altera_mutex_spin_out(void){
	IOWR_ALTERA_AVALON_MUTEX_MUTEX(EE_ALTERA_MUTEX_BASE, EE_CURRENTCPU << ALTERA_AVALON_MUTEX_MUTEX_OWNER_OFST);
}


/* This is used to raise an Interrupt on another CPU */
__INLINE__ void __ALWAYS_INLINE__ EE_hal_IRQ_interprocessor(EE_UREG cpu){
  IOWR_ALTERA_AVALON_PIO_DATA(EE_IPIC_OUTPUT_BASE, 1<<cpu);
  IOWR_ALTERA_AVALON_PIO_DATA(EE_IPIC_OUTPUT_BASE, 0);
}

/* This is used to signal that the interprocessor interrupt on the current CPU has been handled */
__INLINE__ void __ALWAYS_INLINE__ EE_hal_IRQ_interprocessor_served(EE_UINT8 cpu){
  IOWR_ALTERA_AVALON_PIO_EDGE_CAP(EE_IPIC_INPUT_BASE, 0)   ;
}

//////////////////*************************************//////////////////
//////////////////*************************************//////////////////
//////////////////*************************************//////////////////
//////////////////*************************************//////////////////
//////////////////*************************************//////////////////
//////////////////*************************************//////////////////


__INLINE__ void __ALWAYS_INLINE__ EE_hal_spin_in_int(EE_TYPESPIN m){
	register EE_UINT32 new_lock_value;
	register volatile EE_UINT32 *wait_address;
	register EE_UINT32 what_means_locked;
	
	new_lock_value = ((EE_UINT32)&EE_hal_spin_value[m][EE_CURRENTCPU]) | EE_hal_spin_value[m][EE_CURRENTCPU];
	EE_altera_mutex_spin_in();
		wait_address = (EE_UINT32 *)EE_hal_spin_status[m];
		EE_hal_spin_status[m] = new_lock_value;
	EE_altera_mutex_spin_out();

	what_means_locked = ((EE_UINT32)wait_address) & 1;
	wait_address = (EE_ADDR) ( ((EE_UINT32)wait_address) & 0xFFFFFFFE );
	while (*wait_address == what_means_locked);
}

__INLINE__ void __ALWAYS_INLINE__ EE_hal_spin_out_int(EE_TYPESPIN m){
	EE_hal_spin_value[m][EE_CURRENTCPU] = !EE_hal_spin_value[m][EE_CURRENTCPU];
}

#include "kernel/oo/inc/ee_common.h"

//#include "SPIN_PRIO.h"

//extern const EE_TYPEPRIO EE_MAX_prio;
//extern EE_TYPEPRIO EE_sys_ceiling;
__INLINE__ void __ALWAYS_INLINE__ EE_hal_spin_in(EE_TYPESPIN m){	
	spin_lock=0;
	
	EE_altera_mutex_spin_in();
        if((*(EE_UINT32 *)TailQ[m]) != 0xa0){
			spin_lock=1;
            *(EE_UINT32 *)TailQ[m]=GlobalTaskID[EE_stkfirst];
        }
        ResourceQ[m][EE_CURRENTCPU]=GlobalTaskID[EE_stkfirst];
        TailQ[m]=(EE_UINT32)&ResourceQ[m][EE_CURRENTCPU];
    EE_altera_mutex_spin_out();
	
	
	
					
	if(spin_lock==1){
		EE_sys_ceiling|=EE_th_spin_prio[EE_stkfirst];
		EE_sys_ceiling&=~0x80;
	}
	
	
	
	EE_hal_IRQ_end_primitive();
	
		
	while (spin_lock !=0){;;}		
}

#define RN_ReleaseResource 64

#include "ee_internal.h"
__INLINE__ void __ALWAYS_INLINE__ EE_hal_spin_out(EE_TYPESPIN m){	

    EE_UINT32 task2notify=0;
		
    EE_altera_mutex_spin_in();	
        task2notify=ResourceQ[m][EE_CURRENTCPU];
        ResourceQ[m][EE_CURRENTCPU]=0xa0;
    EE_altera_mutex_spin_out();
		
    if(task2notify!=GlobalTaskID[EE_stkfirst]){	
		register EE_TYPERN_PARAM par;
		par.pending = 1;
		EE_rn_send(task2notify, RN_ReleaseResource, par );
	}
}	




//////////////////*************************************//////////////////
//////////////////*************************************//////////////////
//////////////////*************************************//////////////////
//////////////////*************************************//////////////////
//////////////////*************************************//////////////////
//////////////////*************************************//////////////////



#endif
