#ifndef APP_STANDBY_H_
#define APP_STANDBY_H_

#include "Std_Types.h"

/* Set to STD_ON only after completing docs/standby-rti.md and regenerating RTD. */
#define APP_STANDBY_ENABLE (STD_ON)

#define APP_STANDBY_RTI_CLOCK_HZ 48000U
#define APP_STANDBY_DELAY_SECONDS 10U

typedef enum
{
    APP_STANDBY_BOOT = 0,
    APP_STANDBY_DISABLED,
    APP_STANDBY_PREPARING,
    APP_STANDBY_CLOCK_READY,
    APP_STANDBY_ARMED,
    APP_STANDBY_REQUESTED,
    APP_STANDBY_CLOCK_FAILED,
    APP_STANDBY_ARM_FAILED,
    APP_STANDBY_UNEXPECTED_RETURN,
    APP_STANDBY_QUIESCE_FAILED
} App_Standby_StageType;

typedef struct
{
    uint32 modeStat;
    uint32 resetDes;
    uint32 resetFes;
    uint32 wkpuWisr;
    boolean fromStandby;
} App_Standby_BootInfoType;

extern volatile App_Standby_StageType App_Standby_Stage;
extern volatile App_Standby_BootInfoType App_Standby_BootInfo;

/* Call first in main(), before ALL_COMPONENT_INIT(). */
void App_Standby_CaptureBootInfo(void);
/* Call after normal peripheral initialization, before starting UART RX. */
void App_Standby_ReportBootInfo(void);
/* Main context only. A successful normal Standby wake-up starts at reset. */
void App_Standby_Enter(void);
/* Configure this as the GPT RTI notification. */
void App_Standby_RtiNotification(void);

#endif
