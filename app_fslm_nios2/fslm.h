#ifndef FSLM_H_
#define FSLM_H_

//#include "ee_internal.h" 
#include "ee.h"

extern EE_UINT32 pollingVar[4]; 
extern EE_UINT32 HeadQueue[10];           //HQ[#resources]
extern EE_UINT32 TailQueue[10];           //TQ[#resources]
extern EE_UINT32 ResourceQueue[10][4];    //RQ[#resources][#cores]

    
extern int spin_lock;
extern int Task_spin;
extern int Task_pre;
    
    
   

///////////////////////////////////////////////////

    extern EE_UINT32 var[4];
    EE_UINT32 EE_SHARED_DATA(var)[4] ;
    extern EE_UINT32 * Pvar;
    EE_UINT32 * const EE_SHARED_DATA(Pvar) = {var};  
 
    
///////////////////////////////////////////////////

    extern EE_UINT32 TailQ[10];
    EE_UINT32 EE_SHARED_DATA(TailQ)[10] ;
    extern EE_UINT32* PTailQ;
    EE_UINT32 * const EE_SHARED_DATA(PTailQ) = {TailQ};

///////////////////////////////////////////////////


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
    
    
    EE_UINT32 EE_SHARED_DATA(RQ_cpu0)[10];
    EE_UINT32 EE_SHARED_DATA(RQ_cpu1)[10];
    EE_UINT32 EE_SHARED_DATA(RQ_cpu2)[10];
    EE_UINT32 EE_SHARED_DATA(RQ_cpu3)[10];
    EE_UINT32 EE_SHARED_DATA(RQ_cpu4)[10];
    EE_UINT32 EE_SHARED_DATA(RQ_cpu5)[10];
    EE_UINT32 EE_SHARED_DATA(RQ_cpu6)[10];
    EE_UINT32 EE_SHARED_DATA(RQ_cpu7)[10];
    EE_UINT32 EE_SHARED_DATA(RQ_cpu8)[10];
    EE_UINT32 EE_SHARED_DATA(RQ_cpu9)[10];

    extern EE_UINT32 * ResourceQ[10];

    EE_UINT32 * const EE_SHARED_DATA(ResourceQ)[10] = {
        RQ_cpu0,
        RQ_cpu1,
        RQ_cpu2,
        RQ_cpu3,
        RQ_cpu4,
        RQ_cpu5,
        RQ_cpu6,
        RQ_cpu7,
        RQ_cpu8,
        RQ_cpu9,
    };

    ///////////////////////////////////////////////////};
    extern EE_UINT32 MQ_cpu0[NUM_MEASUREMENT_PARAMS];
    extern EE_UINT32 MQ_cpu1[NUM_MEASUREMENT_PARAMS];
    extern EE_UINT32 MQ_cpu2[NUM_MEASUREMENT_PARAMS];
    extern EE_UINT32 MQ_cpu3[NUM_MEASUREMENT_PARAMS];
    
    EE_UINT32 EE_SHARED_DATA(MQ_cpu0)[NUM_MEASUREMENT_PARAMS];
    EE_UINT32 EE_SHARED_DATA(MQ_cpu1)[NUM_MEASUREMENT_PARAMS];
    EE_UINT32 EE_SHARED_DATA(MQ_cpu2)[NUM_MEASUREMENT_PARAMS];
    EE_UINT32 EE_SHARED_DATA(MQ_cpu3)[NUM_MEASUREMENT_PARAMS];
    
    extern EE_UINT32 * MeasureQ[NUM_CPU];
    
    EE_UINT32 * const EE_SHARED_DATA(MeasureQ)[NUM_CPU] = {
        MQ_cpu0,
        MQ_cpu1,
        MQ_cpu2,
        MQ_cpu3,
    };

    ///////////////////////////////////////////////////};



#endif /*FSLM_H_*/
