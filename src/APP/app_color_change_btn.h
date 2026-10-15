/*
 * app_color_change_btn.h
 *
 *  Created on: 2026. 9. 17.
 *      Author: David.Kang
 */

#ifndef _APP_COLOR_CHANGE_BTN_H_
#define _APP_COLOR_CHANGE_BTN_H_

#include "../APP_CFG.h"
#include "Std_Types.h"
/* RTD ICU callback: records an event only; no output or logging. */
void ISR_BTN_Handler(void);
/* Main context only. Handles at most one event per call, preserving the
 * existing OFF -> GREEN -> BLUE sequence. No additional debounce. */
void App_ColorButton_Process(void);
/* Number of events dropped after the 32-event pending capacity is reached. */
uint32 App_ColorButton_GetDroppedEvents(void);

#endif
