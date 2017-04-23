
# task parameters for tasks accessing resources
# in terms of us
# GCS_MIN = 5
# GCS_MAX = 500
# NCS	= 60
# RQ1 = 8
# RQ2	= 8
# AC1	= 5
# AC2 = 9
# RL1 = 8
# RL2 = 10
# RL3 = 12
# RL4 = 15
# AJ = 1
#
# MSRP_RQ = 6
# MSRP_RL = 6
#
# MAX_SIMULATION_TIME = 1000
#
# T3_CT   = 100

# In terms of cycles
GCS_MIN = 5
GCS_MAX = 5000
NCS	= 3000
RQ1 = 395
RQ2	= 403
AC1	= 251
AC2 = 424
RL1 = 383
RL2 = 492
RL3 = 556
RL4 = 729
AJ = 1

MSRP_RQ = 306
MSRP_RL = 270

MAX_SIMULATION_TIME = 100000

T3_CT   = 5000


def calculateWindows(gcs):

    #            T1 spin start  T0 GCS+OH end
    windowCP1 = ((NCS + AJ + RQ2), (NCS + RQ1 + gcs + RL2))
    #            T1 GCS+OH end
    windowCP2 = ((windowCP1[1] + AC2 + gcs + RL3), MAX_SIMULATION_TIME)

    windowHP = ((windowCP1[1] + AC1 + gcs + RL1), MAX_SIMULATION_TIME)

    windowMSRP = ((NCS + MSRP_RQ + gcs + MSRP_RL + gcs + MSRP_RL), MAX_SIMULATION_TIME)

    activationT3 = NCS + AJ + RQ2

    return windowCP1, windowCP2, windowHP, windowMSRP, activationT3

def responseCP(windowCP1, windowCP2, at3):


    t3rt = 0
    returnValue = 0
    for i in range(MAX_SIMULATION_TIME):
        if (i>windowCP1[0]) and (i<=windowCP1[1]):
            t3rt += 1
        elif (i>windowCP2[0]) and (i<=windowCP2[1]):
            t3rt += 1
        if (t3rt>= T3_CT):
            returnValue = i - at3
            break

    return returnValue


def responseHP(windowHP, at3):
    t3rt = 0
    returnValue = MAX_SIMULATION_TIME
    for i in range(MAX_SIMULATION_TIME):
        if (i > windowHP[0]) and (i <= windowHP[1]):
            t3rt += 1
        if (t3rt >= T3_CT):
            returnValue = i - at3
            break

    return returnValue

def responseMSRP(windowMSRP, at3):
    t3rt = 0
    returnValue = MAX_SIMULATION_TIME
    for i in range(MAX_SIMULATION_TIME):
        if (i > windowMSRP[0]) and (i <= windowMSRP[1]):
            t3rt += 1
        if (t3rt >= T3_CT):
            returnValue = i - at3
            break

    return returnValue

def t3rtAnalysis():

    for gcs in range(GCS_MIN,GCS_MAX):
        cp1, cp2, hp, msrp, at3 = calculateWindows(gcs)
        print(gcs,responseCP(cp1,cp2,at3),responseHP(hp,at3),responseMSRP(msrp,at3))

if __name__ == "__main__":
    t3rtAnalysis()