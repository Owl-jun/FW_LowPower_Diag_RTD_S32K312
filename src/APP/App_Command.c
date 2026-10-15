#include "App_Command.h"
#include "../APP_CFG.h"
#include "App_Standby.h"
#include "../Uart/App_Uart.h"
#include "../PWM/App_Pwm.h"

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
        (void)App_Uart_Send("Collect Command Key, Turn On Red LED");
        App_Pwm_On_RedLed(CFG_ON);
    }
    else if (command == 1U)
    {
        App_Standby_Enter();
    }
}

void App_Command_Process(void)
{
    uint8 byte;
    uint32 consumed;

    for (consumed = 0U; consumed < APP_COMMAND_RX_BUDGET; consumed++)
    {
        if (App_Uart_Read(&byte, 1U) == 0U) { break; }

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
