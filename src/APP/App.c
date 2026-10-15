#include "App.h"
#include "App_Command.h"
#include "App_Standby.h"
#include "../platform/Platform_Init.h"
#include "../PWM/App_Pwm.h"
#include "../platform/Uart_Port.h"
#include "../GPT/App_Gpt.h"
#include "../WDG/App_Wdg.h"

void App_Init(void)
{

#if (BUILD_MODE == BM_RELEASE)
	static uint32_t u32BaseTS;
	uint32_t		u32CurTS;
#endif

    /* Preserve wakeup/reset evidence before drivers modify status flags. */
    App_Standby_CaptureBootInfo();
    Plat_InitHardware();
    App_Standby_ReportBootInfo();

    if (Plat_Uart_Recv_Async() != E_OK)
    {
#if (BUILD_MODE == BM_RELEASE)
    	u32BaseTS = App_Gpt_Get_Elapsed_Sec();
#endif
        for (;;) {
#if (BUILD_MODE == BM_RELEASE)
        	u32CurTS = App_Gpt_Get_Elapsed_Sec();
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
    Plat_Uart_RxProcess();
    App_Command_Process();
    App_Pwm_Process();
    App_Gpt_Send_Time_Uart();

#if (BUILD_MODE == BM_RELEASE)
	#if (WATCHDOG_ENABLE == CFG_ON)
		App_Wdg_Refresh();
	#endif
#endif
}
