/*
 * Uart_Port.h
 *
 *  Created on: 2026. 9. 21.
 *      Author: David.Kang
 */

#ifndef UART_PORT_H
#define UART_PORT_H

#include "Std_Types.h"
#include "Uart_Types.h"

/* Blocking send of a NUL-terminated string followed by CRLF.
 * Caller supplies text without CRLF. Calls must not overlap. */
Std_ReturnType Plat_Uart_Send(const char *text);

/* Call once after Uart_Init and Platform_Init. Check E_OK. */
Std_ReturnType Plat_Uart_Recv_Async(void);

/* Nonblocking main-loop read. Returns byte count; does not add a NUL byte.
 * Call regularly. Message boundaries (newline/length) belong to the caller. */
uint32 Plat_Uart_Read(uint8 *data, uint32 capacity);

/* Main context only. Recover reception after a UART error.
 * Command parsing belongs to App_Command_Process. */
void Plat_Uart_RxProcess(void);

uint32 Plat_Uart_GetRxDropped(void);
uint32 Plat_Uart_GetRxErrors(void);
void Plat_Uart_Callback(const uint8 channel, const Uart_EventType event);

#endif
