#include "ee_internal.h"

EE_TID EE_rq2stk_exchange(void){
	EE_INT8 x;    // the first non-empty queue 
	EE_TID temp;   // the TID to be inserted in the top of the stack
	EE_TYPEPAIR y; // used to free the descriptor
	x = EE_rq_lookup[EE_rq_bitmask];
	// now x contains the highest priority non-empty queue number
	// get the TID to insert in the stacked queue
	temp = EE_rq_pairs_tid[EE_rq_queues_head[x]];
	y = EE_rq_queues_head[x];// free the descriptor 
	EE_rq_queues_head[x] = EE_rq_pairs_next[EE_rq_queues_head[x]];
	EE_rq_pairs_next[y] = EE_rq_free;
	EE_rq_free = y;

	if (EE_rq_queues_head[x] == -1) {
		EE_rq_queues_tail[x] = -1;
		EE_rq_bitmask &= ~(1<<x);// reset the (x)th bit in the bitfield
	}

	// insert the extracted task on the top of the stack
	EE_th_next[temp] = EE_stkfirst;
	EE_stkfirst = temp;
	return temp;
}
