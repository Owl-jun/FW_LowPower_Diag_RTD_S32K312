#include "App.h"
#include "App_Command.h"
#include "app_color_change_btn.h"
#include "App_Standby.h"
#include "App_Pwm.h"
#include "App_Telemetry.h"
#include "App_Log.h"
#include "../platform/Platform_Init.h"
#include "../platform/Uart_Port.h"
#include "../platform/Timebase.h"
#include "../platform/Wdg_Port.h"

#include "Wkpu_Ip.h"
#include "S32K312_MC_ME.h"
#include "S32K312_MC_RGM.h"
#include "S32K312_WKPU.h"

void App_Init(void)
{

#if (BUILD_MODE == BM_RELEASE)
	static uint32_t u32BaseTS;
	uint32_t		u32CurTS;
#endif

    /* Preserve wakeup/reset evidence before drivers modify status flags. */
#if (STANDBY_ENABLE == CFG_ENABLE)

#ifdef TEST_WAKE_UP
	volatile uint32_t MODE_STAT = IP_MC_ME->MODE_STAT;
	volatile uint32_t WISR = IP_WKPU->WISR;
	volatile uint32_t WISR_64 = IP_WKPU->WISR_64;
	uint8_t SW2_WAKEUP = Wkpu_Ip_GetInputState(0U, 42U);
	uint8_t HWCH10_WAKEUP = Wkpu_Ip_GetInputState(0U, 10U);
#endif
	App_Standby_CaptureBootInfo();
#endif
    Plat_InitHardware();

#ifdef TEST_WAKE_UP
    App_Log_Printf("MODE_STAT : %08lx , WISR(Expanded) : %08lx%08lx" , MODE_STAT , WISR, WISR_64);
    App_Log_Printf("WAKEUP(SW2) : %d , HWCH10_WAKEUP : %d" , SW2_WAKEUP , HWCH10_WAKEUP);
    WISR = IP_WKPU->WISR;
    WISR_64 = IP_WKPU->WISR_64;
    App_Log_Printf("MODE_STAT : %08lx , WISR(Expanded) : %08lx%08lx" , MODE_STAT , WISR, WISR_64);
#endif

    if (Plat_Uart_Recv_Async() != E_OK)
    {
#if (BUILD_MODE == BM_RELEASE)
        u32BaseTS = Plat_Timebase_GetElapsedSec();
#endif
        for (;;) {
#if (BUILD_MODE == BM_RELEASE)
            u32CurTS = Plat_Timebase_GetElapsedSec();
        	if (u32CurTS - u32BaseTS > T_SEC_TIMEOUT_INF_LOOP) {
        		App_Pwm_On_RedLed(CFG_ON);
        		break;
        	}
#else
        	/* RX start failed: inspect UART initialization. */
#endif
		}
    }
}

void App_Process(void)
{
    /* Preserve the existing superloop order. ISR callbacks only provide
     * inputs; command execution remains in main context. */
#if (UART_ENABLE == CFG_ENABLE)
    Plat_Uart_RxProcess();
    App_Command_Process();

#if (TIMEBASE_ENABLE == CFG_ENABLE)
	App_Telemetry_Process();
#endif

#endif

#if (PWM_ENABLE == CFG_ENABLE)
    App_Pwm_Process();
#endif

    App_ColorButton_Process();
    Plat_Wdg_Refresh();
}
