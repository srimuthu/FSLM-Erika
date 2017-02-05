#include "ee_internal.h"

#ifndef __PRIVATE_RQ2STK_EXCHANGE__

EE_TID EE_rq_queryfirst(void){
  EE_INT8 x = EE_rq_lookup[EE_rq_bitmask];
  /* now x contains the highest priority non-empty queue number */
  if (x == -1){return EE_NIL;}
  else{ return EE_rq_pairs_tid[EE_rq_queues_head[x]];}
}
#endif


