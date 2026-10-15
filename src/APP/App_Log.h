#ifndef APP_LOG_H
#define APP_LOG_H

#include "Std_Types.h"

boolean App_Log_IsEnabled(void);
/* Blocking UART output when enabled. OFF returns E_OK without transmitting.
 * E_OK therefore means no logging error, not proof of transmission.
 * Calls must not overlap; this module does not queue or serialize output. */
Std_ReturnType App_Log_Write(const char *text);
/* Bounded formatting (160 bytes including NUL); rejects truncated messages. */
Std_ReturnType App_Log_Printf(const char *format, ...)
    __attribute__((format(printf, 1, 2)));

#endif
