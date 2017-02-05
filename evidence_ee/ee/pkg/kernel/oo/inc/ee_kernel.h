#ifndef __INCLUDE_OO_KERNEL_H__
#define __INCLUDE_OO_KERNEL_H__

#include "kernel/oo/inc/ee_common.h"


#define INVALID_TASK EE_NIL
#define DeclareTask(t) void Func##t(void)
#define TASK(t) void Func##t(void)
#define DeclareResource(ResourceIdentifier) extern EE_TID EE_th_next[] /* void! */
#define DeclareEvent(EventIdentifier) extern EE_TID EE_th_next[] /* void! */

StatusType EE_oo_ActivateTask(TaskType TaskID);
StatusType EE_oo_TerminateTask(void);
StatusType EE_oo_GetResource(ResourceType ResID);
StatusType EE_oo_ReleaseResource(ResourceType ResID);
void EE_oo_StartOS(AppModeType Mode);

#endif
