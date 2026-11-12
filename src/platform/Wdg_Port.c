/*
 * Wdg_Port.c
 *
 *  Created on: 2026. 9. 22.
 *      Author: David.Kang
 */

#include "Wdg_Port.h"
#include "../APP_CFG.h"
#include "Swt_Ip.h"
#if (WATCHDOG_ENABLE == CFG_ENABLE)
#include "Wdg_43_Instance0.h"
#if (UART_ENABLE == CFG_ENABLE) && (UART_LOG_ENABLE == CFG_ENABLE)
#include "Uart_Port.h"
#endif
#endif

void Plat_Wdg_Init(void)
{
#if (WATCHDOG_ENABLE == CFG_ENABLE)
    Wdg_43_Instance0_Init(NULL_PTR);
#endif
}

Std_ReturnType Plat_Wdg_Deinit(void)
{
    if ((IP_SWT_0->CR & SWT_CR_WEN_MASK) != 0U)
    {
        if (Swt_Ip_Deinit(0U) != SWT_IP_STATUS_SUCCESS)
        {
            return E_NOT_OK;
        }
    }

    return ((IP_SWT_0->CR & SWT_CR_WEN_MASK) == 0U) ? E_OK : E_NOT_OK;
}

void Plat_Wdg_Refresh(void)
{
#if (WATCHDOG_ENABLE == CFG_ENABLE)
    if ((IP_SWT_0->CR & SWT_CR_WEN_MASK) != 0U)
    {
        Wdg_43_Instance0_Service();
    }
#endif
}

void Plat_Wdg_Callback(void)
{
#if (WATCHDOG_ENABLE == CFG_ENABLE)
#if (UART_ENABLE == CFG_ENABLE) && (UART_LOG_ENABLE == CFG_ENABLE)
	(void)Plat_Uart_Send("Watchdog Timeout Occured");
#endif
    Wdg_43_Instance0_Service();
	for ( ; ; ) { /* WDG TIMEOUT OCCURED !! */ }
#endif
}
