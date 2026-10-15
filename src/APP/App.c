#include "App.h"
#include "App_Command.h"
#include "App_Standby.h"
#include "../INIT/init.h"
#include "../PWM/App_Pwm.h"
#include "../Uart/App_Uart.h"
#include "../GPT/App_Gpt.h"
#include "../WDG/App_Wdg.h"

void App_Init(void)
{
	static uint32_t u32BaseTS;
	uint32_t		u32CurTS;

    /* Preserve wakeup/reset evidence before drivers modify status flags. */
    App_Standby_CaptureBootInfo();
    ALL_COMPONENT_INIT();
    App_Standby_ReportBootInfo();

    if (App_Uart_Recv_Async() != E_OK)
    {
    	u32BaseTS = App_Gpt_Get_Elapsed_Sec();
        for (;;) {
#if (BUILD_MODE == BM_RELEASE)
        	u32CurTS = App_Gpt_Get_Elapsed_Sec();
        	if (u32CurTS - u32BaseTS > T_SEC_TIMEOUT_INF_LOOP) {
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
    App_Uart_RxProcess();
    App_Command_Process();
    App_Pwm_Process();
    App_Gpt_Send_Time_Uart();

#if (BUILD_MODE == BM_RELEASE)
	#if (WATCHDOG_ENABLE == CFG_ON)
		App_Wdg_Refresh();
	#endif
#endif
}
