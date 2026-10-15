/*
 * Drv_Pwm.h
 *
 *  Created on: 2026. 9. 21.
 *      Author: David.Kang
 */

#ifndef PWM_APP_PWM_H_
#define PWM_APP_PWM_H_
#include "../APP_CFG.h"
#include "Std_Types.h"
#include <stdio.h>

#if (PWM_ENABLE == CFG_ENABLE)
void App_Pwm_On_RedLed(uint8_t cmd);
void App_Pwm_Process(void);
#endif

#endif /* PWM_APP_PWM_H_ */
