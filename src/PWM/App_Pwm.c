/*
 * Drv_Pwm.c
 *
 *  Created on: 2026. 9. 21.
 *      Author: David.Kang
 */

#include "App_Pwm.h"

#include "Dio.h"
#include "Pwm.h"

#include "../Uart/App_Uart.h"

#define DEBOUNCE_CNT 					(10000U)
#define DUTY_CYCLE_100_PERCENT			(0x8000U)
#define DUTY_CONVERT_LOGIC(u8BtnCnt)	(((DUTY_CYCLE_100_PERCENT * (u8BtnCnt * 10U))) / 100U)

void App_Pwm_On_RedLed(uint8_t cmd)
{
	if (cmd) { Pwm_SetDutyCycle(PwmChannel_0, DUTY_CYCLE_100_PERCENT); }
	else { Pwm_SetDutyCycle(PwmChannel_0, 0U); }
}

void App_Pwm_Process(void)
{
	static uint8_t u8BtnCnt = 0U;
	static uint32_t u32Debounce = 0U;
	static uint8_t u8Stop = 0U;
	Dio_LevelType tState;

	/* Polling Btn State */
	tState = Dio_ReadChannel(DioConf_DioChannel_PTB19);

	if (!u8Stop)
	{
		if (tState)
		{
			u32Debounce++;

			if (u32Debounce >= DEBOUNCE_CNT)
			{
				u8BtnCnt = (u8BtnCnt + 1U) % (11U);

				Pwm_SetDutyCycle(PwmChannel_0, DUTY_CONVERT_LOGIC(u8BtnCnt));
				Pwm_SetDutyCycle(PwmChannel_1, DUTY_CONVERT_LOGIC(u8BtnCnt));
				char msg[32];

				/* Percent in hundredths, rounded; no float printf support needed. */
				uint32_t duty = (DUTY_CONVERT_LOGIC(u8BtnCnt) * 10000U
						+ DUTY_CYCLE_100_PERCENT / 2U) / DUTY_CYCLE_100_PERCENT;

				snprintf(msg, sizeof(msg), "Cur Duty Cycle : %u.%02u%%",
						(unsigned int)(duty / 100U), (unsigned int)(duty % 100U));
				(void)App_Uart_Send(msg);

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

