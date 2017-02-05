#include "cpu/nios2/inc/ee_compiler.h"

	.global	EE_nios2_terminate_task
	.global EE_nios2_terminate_savestk
	
	.text

	// r4 = &terminate_data[t]
	// r5 = terminate_real_th_body[t]()
	.type	EE_nios2_terminate_savestk, @function
EE_nios2_terminate_savestk:
	// save some registers:	
	// - r16-r23 that are the Callee-Saved General-Purpose Registers
	// - fp, ra --> because they were used in longjmp
	// - gp should not be saved, it is used by the compiler!
	// Please note that the interrupt are ENABLED

	addi	sp, sp, -40
        stw     r16, 36(sp)
        stw     r17, 32(sp)
        stw     r18, 28(sp)	
        stw     r19, 24(sp)
        stw     r20, 20(sp)
        stw     r21, 16(sp)
        stw     r22, 12(sp)
        stw     r23, 8(sp)
        stw     fp, 4(sp)
        stw     ra, 0(sp)
	
	
	// save sp in terminate_data[t]
	stw	sp, 0(r4)
	
	// jmp terminate_real_th_body[t]
	jmp	r5
	
	.size	EE_nios2_terminate_savestk, . - EE_nios2_terminate_savestk

	// r4 = &terminate_data[t]
	.type	EE_nios2_terminate_task, @function
EE_nios2_terminate_task:
	// reload sp from terminate_data[t]
	ldw	sp, 0(r4)

	// reload the registers saved before
        ldw     r16, 36(sp)
        ldw     r17, 32(sp)
        ldw     r18, 28(sp)
		ldw     r19, 24(sp)
        ldw     r20, 20(sp)
        ldw     r21, 16(sp)
        ldw     r22, 12(sp)
        ldw     r23, 8(sp)
        ldw     fp, 4(sp)
        ldw     ra, 0(sp)
		addi	sp, sp, 40

	// return back to the caller (in tstub.c!!!)
	ret
	.size	EE_nios2_terminate_task, . - EE_nios2_terminate_task
	
	.end
