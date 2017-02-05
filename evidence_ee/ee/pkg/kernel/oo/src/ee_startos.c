#include "ee_internal.h"

StatusType EE_oo_StartOS(AppModeType Mode){ 
	if (EE_cpu_startos()) { return E_OS_SYS_INIT;}

	/* in this case, there is no error or the error is ignored */
	EE_cpu_startos();
	return E_OK;
}
