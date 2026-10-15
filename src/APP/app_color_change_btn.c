/*
 * app_color_change_btn.c
 *
 *  Created on: 2026. 9. 17.
 *      Author: David.Kang
 */

#include "app_color_change_btn.h"

#include "Platform_Types.h"
#include "Dio.h"
#include "../platform/Pwm_Port.h"

#include "App_Log.h"

typedef enum ColorType
{
	COL_OFF = 0U,
	COL_GREEN,
	COL_BLUE,
	COL_MAXCOUNT
} ColorType_e;

/* Single producer (ICU callback), single consumer (main loop).
 * Each context writes only its own counter; neither clears the other's.
 * Unsigned subtraction also works when the counters wrap. */
#define APP_COLOR_EVENT_CAPACITY 32U
static volatile uint32 producedEvents;
static volatile uint32 consumedEvents;
static volatile uint32 droppedEvents;

void ISR_BTN_Handler(void)
{
    if ((uint32)(producedEvents - consumedEvents) < APP_COLOR_EVENT_CAPACITY)
    {
        producedEvents++;
    }
    else
    {
        droppedEvents++; /* Drop newest event when main cannot keep up. */
    }
}

uint32 App_ColorButton_GetDroppedEvents(void)
{
    return droppedEvents;
}

void App_ColorButton_Process(void)
{
	const char *msg = "LED_OFF";
	static ColorType_e eColType = COL_OFF;

    if (consumedEvents == producedEvents) { return; }
    consumedEvents++;

#if (PWM_ENABLE == CFG_ENABLE)
    Plat_Pwm_SetRedDuty(0U);
#endif

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

	(void)App_Log_Write(msg);

	eColType = (eColType + 1U) % COL_MAXCOUNT;

	return;
}
