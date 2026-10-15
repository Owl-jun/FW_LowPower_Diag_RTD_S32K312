/*
 * main.c
 *
 *  Created on: 2026. 9. 17.
 *      Author: David.Kang
 */

/* INCLUDE */
#include "Mcal.h"
#include "INIT/init.h"
#include "PWM/App_Pwm.h"
#include "Uart/App_Uart.h"
#include "WDG/App_Wdg.h"
#include "GPT/App_Gpt.h"
#include "APP/App_Standby.h"
#include "APP/App_Command.h"


/* MAIN */
int main(void)
{
	App_Standby_CaptureBootInfo();
	/* All Use Peripherals Initialize */
	ALL_COMPONENT_INIT();
	App_Standby_ReportBootInfo();

	/* UART RX Start for receive User Command */
	if (App_Uart_Recv_Async() != E_OK)
	{
		for (;;) { /* RX start failed: inspect UART initialization. */ }
	}

    for(;;)	/* Main Loop */
    {
    	/* UART Rx and Btn(SW2) Input are handled by ISR */

        App_Uart_RxProcess();        /* UART receive recovery */
        App_Command_Process();      /* Parse and execute commands in main */
        App_Pwm_Process();			/* Pwm Control by Btn(SW3) Input (Polling) */
        App_Gpt_Send_Time_Uart(); 	/* Send Elapsed Time Since Boot */

#if WATCHDOG_ENABLE
        App_Wdg_Refresh();			/* Wdg 43 Instance 0 Service */
#endif
    }

    return 0U;
}

