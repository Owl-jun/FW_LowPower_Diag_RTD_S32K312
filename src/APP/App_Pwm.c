/*
 * Drv_Pwm.c
 *
 *  Created on: 2026. 9. 21.
 *      Author: David.Kang
 */

#include "App_Pwm.h"

#if (PWM_ENABLE == CFG_ENABLE)
#include "../platform/Button_Port.h"
#include "../platform/Pwm_Port.h"

#if (UART_LOG_ENABLE == CFG_ENABLE  \
	&& UART_ENABLE == CFG_ENABLE 	)
#include "../platform/Uart_Port.h"
#endif

#define DEBOUNCE_CNT 					(10000U)
#define DUTY_CYCLE_100_PERCENT            (PLAT_PWM_DUTY_FULL_SCALE)
#define DUTY_CONVERT_LOGIC(u8BtnCnt)	(((DUTY_CYCLE_100_PERCENT * (u8BtnCnt * 10U))) / 100U)

void App_Pwm_On_RedLed(uint8_t cmd)
{
	if (cmd) { Plat_Pwm_SetRedDuty(DUTY_CYCLE_100_PERCENT); }
	else { Plat_Pwm_SetRedDuty(0U); }
}

void App_Pwm_Process(void)
{
#if (UART_LOG_ENABLE == CFG_ENABLE  \
	&& UART_ENABLE == CFG_ENABLE 	)
	static char msg[32];
#endif
	static uint8_t u8BtnCnt = 0U;
	static uint32_t u32Debounce = 0U;
	static uint8_t u8Stop = 0U;
	boolean tState;

	/* Polling Btn State */
	tState = Plat_Button_IsPwmInputHigh();

	if (!u8Stop)
	{
		if (tState)
		{
			u32Debounce++;

			if (u32Debounce >= DEBOUNCE_CNT)
			{
				u8BtnCnt = (u8BtnCnt + 1U) % (11U);
				Plat_Pwm_SetPairDuty(DUTY_CONVERT_LOGIC(u8BtnCnt));

#if (UART_LOG_ENABLE == CFG_ENABLE  \
	&& UART_ENABLE == CFG_ENABLE 	)

				/* Percent in hundredths, rounded; no float printf support needed. */
				uint32_t duty = (DUTY_CONVERT_LOGIC(u8BtnCnt) * 10000U
						+ DUTY_CYCLE_100_PERCENT / 2U) / DUTY_CYCLE_100_PERCENT;

				snprintf(msg, sizeof(msg), "Cur Duty Cycle : %u.%02u%%",
						(unsigned int)(duty / 100U), (unsigned int)(duty % 100U));
				(void)Plat_Uart_Send(msg);
#endif

				u8Stop = 1U;
			}
		}
		else
		{
			u8Stop = 0U;
			u32Debounce = 0U;
		}
	}
	else if (u8Stop)
	{
		if (tState)
		{
			/* DO NOTHING */
		}
		else
		{
			u8Stop = 0U;
			u32Debounce = 0U;
		}
	}
}
#endif

