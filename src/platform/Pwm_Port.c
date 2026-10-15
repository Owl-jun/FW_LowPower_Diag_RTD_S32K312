#include "Pwm_Port.h"
#include "Pwm.h"

#if (PWM_ENABLE == CFG_ENABLE)
void Plat_Pwm_SetRedDuty(uint16 duty)
{
    Pwm_SetDutyCycle(PwmChannel_0, duty);
}

void Plat_Pwm_SetPairDuty(uint16 duty)
{
    Pwm_SetDutyCycle(PwmChannel_0, duty);
    Pwm_SetDutyCycle(PwmChannel_1, duty);
}
#endif
