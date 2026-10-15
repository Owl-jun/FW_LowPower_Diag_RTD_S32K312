#ifndef PLATFORM_PWM_PORT_H
#define PLATFORM_PWM_PORT_H

#include "../APP_CFG.h"
#include "Std_Types.h"

#if (PWM_ENABLE == CFG_ENABLE)
/* RTD duty scale: 0 = 0%, 0x8000 = 100%. Caller supplies that range. */
#define PLAT_PWM_DUTY_FULL_SCALE 0x8000U

void Plat_Pwm_SetRedDuty(uint16 duty);
void Plat_Pwm_SetPairDuty(uint16 duty);
#endif

#endif
