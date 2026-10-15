#include "Button_Port.h"
#include "Dio.h"

boolean Plat_Button_IsPwmInputHigh(void)
{
    return (Dio_ReadChannel(DioConf_DioChannel_PTB19) != 0U) ? TRUE : FALSE;
}
