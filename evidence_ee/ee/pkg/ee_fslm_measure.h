/*
ee_fslm_measure.h

Added for defining measurement flags for performance measurements of FSLM.
Author: S.M.N. Balasubramanian
*/

#ifndef __EE_FSLM_MEASURE_H__
#define __EE_FSLM_MEASURE_H__

//Measurement variables
#define NUM_MEASUREMENT_PARAMS	4
#define NUM_CPU					4

extern EE_UINT32 * MeasureQ[NUM_CPU];

/*
	CPU wise arrays
*/
extern EE_UINT32 MQ_cpu0[NUM_MEASUREMENT_PARAMS];
extern EE_UINT32 MQ_cpu1[NUM_MEASUREMENT_PARAMS];
extern EE_UINT32 MQ_cpu2[NUM_MEASUREMENT_PARAMS];
extern EE_UINT32 MQ_cpu3[NUM_MEASUREMENT_PARAMS];


/*Measurement flags - comment or uncomment as required
Number of flags and associated variables should 
be equal to NUM_MEASUREMENT_PARAMS
Each measurement flag has a unique ID
*/

/*
	REMOTE NOTIFICATION MEASUREMENTS
*/
//#define MF_RN_SEND						0		//C0 S0
#define MF_RN_HANDLER					1		//C1 S0 
#define MF_RN_EXECUTE					2		//C1 S1
#define MF_RELEASE2LOCK                 3       //C0 S0

/*
Measurement start template
#ifdef MF_FLAG
    PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
    PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);
    PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE,1);
#endif

#ifdef MF_FLAG	
	PERF_END(PERFORMANCE_COUNTER_0_BASE,1);
    PERF_STOP_MEASURING(PERFORMANCE_COUNTER_0_BASE);
    MeasureQ[EE_CURRENTCPU][MF_FLAG] = perf_get_section_time((void *)PERFORMANCE_COUNTER_0_BASE, 1);
#endif
Measurement end template

*/

#endif
