/*
 * app_color_change_btn.c
 *
 *  Created on: 2026. 9. 17.
 *      Author: David.Kang
 */

#include "../APP/app_color_change_btn.h"

#include "Platform_Types.h"
#include "Siul2_Port_Ip.h"
#include "Dio.h"
#include "Pwm.h"

#include "../platform/Uart_Port.h"

typedef enum ColorType
{
	COL_OFF = 0U,
	COL_GREEN,
	COL_BLUE,
	COL_MAXCOUNT
} ColorType_e;

void ISR_BTN_Handler(void)
{
	const char *msg = "LED_OFF";
	static ColorType_e eColType = COL_OFF;

	Pwm_SetDutyCycle(
			PwmChannel_0,
			0U
			);

	if (eColType == COL_OFF)
	{
		msg = "LED_OFF";

		Dio_WriteChannel(DioConf_DioChannel_PTA30, STD_HIGH);
		Dio_WriteChannel(DioConf_DioChannel_PTA31, STD_HIGH);
	}
	else if (eColType == COL_GREEN)
	{
		msg = "LED_GREEN";

		Dio_WriteChannel(DioConf_DioChannel_PTA30, STD_LOW);
		Dio_WriteChannel(DioConf_DioChannel_PTA31, STD_HIGH);
	}
	else if (eColType == COL_BLUE)
	{
		msg = "LED_BLUE";

		Dio_WriteChannel(DioConf_DioChannel_PTA30, STD_HIGH);
		Dio_WriteChannel(DioConf_DioChannel_PTA31, STD_LOW);
	}

	(void)Plat_Uart_Send(msg);

	eColType = (eColType + 1U) % COL_MAXCOUNT;

	return;
}
