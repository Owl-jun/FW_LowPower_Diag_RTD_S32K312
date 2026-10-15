#include "App_Log.h"
#include "../APP_CFG.h"

#if (UART_LOG_ENABLE == CFG_ENABLE) && (UART_ENABLE == CFG_ENABLE)
#include "../platform/Uart_Port.h"
#include <stdarg.h>
#include <stdio.h>

boolean App_Log_IsEnabled(void) { return TRUE; }

Std_ReturnType App_Log_Write(const char *text)
{
    return Plat_Uart_Send(text);
}

Std_ReturnType App_Log_Printf(const char *format, ...)
{
    char text[160];
    int length;
    va_list args;
    if (format == NULL_PTR) { return E_NOT_OK; }
    va_start(args, format);
    length = vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    if ((length < 0) || ((unsigned int)length >= sizeof(text)))
    {
        return E_NOT_OK;
    }
    return Plat_Uart_Send(text);
}
#else
boolean App_Log_IsEnabled(void) { return FALSE; }
Std_ReturnType App_Log_Write(const char *text)
{
    (void)text;
    return E_OK;
}
Std_ReturnType App_Log_Printf(const char *format, ...)
{
    (void)format;
    return E_OK;
}
#endif
