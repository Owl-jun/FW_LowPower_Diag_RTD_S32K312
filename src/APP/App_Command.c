#include "App_Command.h"
#include "App_Log.h"
#include "../APP_CFG.h"

#if (UART_ENABLE == CFG_ENABLE)
#include "../platform/Uart_Port.h"
#endif

#if (STANDBY_ENABLE == CFG_ENABLE)
#include "App_Standby.h"
#endif

#if (PWM_ENABLE == CFG_ENABLE)
#include "App_Pwm.h"
#endif

#define APP_COMMAND_NONE 0xFFU
#define APP_COMMAND_PAYLOAD_LENGTH 6U
#define APP_COMMAND_RX_BUDGET 32U

/* Preserve the existing wire protocol: 0950809 = red LED,
 * 1950808 = Standby. A mismatching byte discards the partial command. */
static const char * const App_Command_Payloads[] = {"950809", "950808"};
static uint8 App_Command_Id = APP_COMMAND_NONE;
static uint8 App_Command_Position;

static void App_Command_Execute(uint8 command)
{
    if (command == 0U)
    {
        (void)App_Log_Write("Collect Command Key, Turn On Red LED");
#if (PWM_ENABLE == CFG_ENABLE)
        App_Pwm_On_RedLed(CFG_ON);
#endif
    }
    else if (command == 1U)
    {
#if (STANDBY_ENABLE == CFG_ENABLE)
        App_Standby_Enter();
#endif
    }
}

void App_Command_Process(void)
{
    uint8 byte;
    uint32 consumed;

    for (consumed = 0U; consumed < APP_COMMAND_RX_BUDGET; consumed++)
    {
#if (UART_ENABLE == CFG_ENABLE)
        if (Plat_Uart_Read(&byte, 1U) == 0U) { break; }
#endif
        if (App_Command_Id == APP_COMMAND_NONE)
        {
            if ((byte == '0') || (byte == '1'))
            {
                App_Command_Id = (uint8)(byte - '0');
                App_Command_Position = 0U;
            }
        }
        else if (byte == (uint8)App_Command_Payloads[App_Command_Id][App_Command_Position])
        {
            App_Command_Position++;
            if (App_Command_Position == APP_COMMAND_PAYLOAD_LENGTH)
            {
                uint8 command = App_Command_Id;
                App_Command_Id = APP_COMMAND_NONE;
                App_Command_Position = 0U;
                App_Command_Execute(command);
                break;
            }
        }
        else
        {
            App_Command_Id = APP_COMMAND_NONE;
            App_Command_Position = 0U;
        }
    }
}
