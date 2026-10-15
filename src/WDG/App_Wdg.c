/*
 * App_Wdg.c
 *
 *  Created on: 2026. 9. 22.
 *      Author: David.Kang
 */

#include "App_Wdg.h"
#include "Wdg_43_Instance0.h"
#include "../INIT/init.h"
#include "../Uart/App_Uart.h"

// #define DEBUG_WDG_TIMEOUT

void App_Wdg_Refresh(void)
{
	Wdg_43_Instance0_Service();
}

void Wdg_Callback(void)
{
	(void)App_Uart_Send("Watchdog Timeout Occured");
#if WATCHDOG_ENABLE
		Wdg_43_Instance0_Service();
#endif
	for ( ; ; ) { /* WDG TIMEOUT OCCURED !! */ }
}
