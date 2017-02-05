#include "ee_internal.h"

void EE_rq_insert(EE_TID t){
	EE_TYPEPAIR temp;
	EE_TYPEPRIO p;

	p = EE_rq_link[t];

	temp = EE_rq_free;
	EE_rq_free = EE_rq_pairs_next[EE_rq_free];

	EE_rq_pairs_tid[temp] = t;
	EE_rq_pairs_next[temp] = -1;

	if (EE_rq_queues_tail[p] == -1) {
		EE_rq_bitmask |= EE_th_ready_prio[t];
		EE_rq_queues_head[p] = temp;
	} else {
		EE_rq_pairs_next[EE_rq_queues_tail[p]] = temp;
	}
	EE_rq_queues_tail[p] = temp;
}
