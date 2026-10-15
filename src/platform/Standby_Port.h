#ifndef PLAT_STANDBY_H_
#define PLAT_STANDBY_H_

#include "Std_Types.h"



#define PLAT_STANDBY_RTI_CLOCK_HZ 48000U
#define PLAT_STANDBY_DELAY_SECONDS 10U

typedef enum
{
    PLAT_STANDBY_BOOT = 0,
    PLAT_STANDBY_DISABLED,
    PLAT_STANDBY_PREPARING,
    PLAT_STANDBY_CLOCK_READY,
    PLAT_STANDBY_ARMED,
    PLAT_STANDBY_REQUESTED,
    PLAT_STANDBY_CLOCK_FAILED,
    PLAT_STANDBY_ARM_FAILED,
    PLAT_STANDBY_UNEXPECTED_RETURN,
    PLAT_STANDBY_QUIESCE_FAILED
} Plat_Standby_StageType;

typedef struct
{
    uint32 modeStat;
    uint32 resetDes;
    uint32 resetFes;
    uint32 wkpuWisr;
    boolean fromStandby;
    boolean fromRti;
} Plat_Standby_BootInfoType;

extern volatile Plat_Standby_StageType Plat_Standby_Stage;
extern volatile Plat_Standby_BootInfoType Plat_Standby_BootInfo;

/* Capture before peripheral initialization modifies reset/wakeup flags. */
void Plat_Standby_CaptureBootInfo(void);
boolean Plat_Standby_IsAvailable(void);
/* Main context. A successful wake restarts at reset. Failures after hardware
 * preparation halt with Plat_Standby_Stage rather than resume normal work. */
void Plat_Standby_Enter(void);

#endif
