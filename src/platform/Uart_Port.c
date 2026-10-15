/*
 * Uart_Port.c
 *
 *  Created on: 2026. 9. 21.
 *      Author: David.Kang
 */

#include "Uart_Port.h"

#if (UART_ENABLE == CFG_ENABLE)
#include "CDD_Uart.h"
#include "Lpuart_Uart_Ip.h"
#include "IntCtrl_Ip.h"

#define RX_BUFFER_SIZE 	256U
#define UART_LIMIT_LEN	4096U

/* Single ISR producer, single main-loop consumer. Capacity: 255 bytes. */
static uint8 rxByte;
static volatile uint8 rxBuf[RX_BUFFER_SIZE];
static volatile uint32 rxHead;
static volatile uint32 rxTail;
static volatile uint32 rxDropped;
static volatile uint32 rxErrors;
static boolean rxStarted;

Std_ReturnType Plat_Uart_Recv_Async(void)
{
	Std_ReturnType result = Uart_AsyncReceive(0U, &rxByte, 1U);
	if (result == E_OK) { rxStarted = TRUE; }
	return result;
}

uint32 Plat_Uart_Read(uint8 *data, uint32 capacity)
{
	uint32 count = 0U;
	if (data == NULL_PTR) { return 0U; }
	while ((count < capacity) && (rxTail != rxHead))
	{
		data[count++] = rxBuf[rxTail];
		rxTail = (rxTail + 1U) % RX_BUFFER_SIZE;
	}
	return count;
}

void Plat_Uart_RxProcess(void)
{
	uint32 remaining;
	/* Main context only. Restart after RTD has stopped RX on an error. */
	if (rxStarted)
	{
		IntCtrl_Ip_DisableIrq(LPUART6_IRQn);
		if (Uart_GetStatus(0U, &remaining, UART_RECEIVE) != UART_STATUS_OPERATION_ONGOING)
		{
			(void)Uart_AsyncReceive(0U, &rxByte, 1U);
		}
		IntCtrl_Ip_EnableIrq(LPUART6_IRQn);
	}

}

void Plat_Uart_Callback(const uint8 channel, const Uart_EventType event)
{
	if (channel != 0U) { return; }
	if (event == UART_EVENT_RX_FULL)
	{
		uint32 next = (rxHead + 1U) % RX_BUFFER_SIZE;

		if (next != rxTail)
		{
			rxBuf[rxHead] = rxByte;
			rxHead = next;
		}
		else { rxDropped++; } /* Drop newest byte, keep reception active. */
		/* Extend RX before the RTD handler finishes/disables reception. */
		Lpuart_Uart_Ip_SetRxBuffer(6U, &rxByte, 1U);
	}
	else if (event == UART_EVENT_ERROR) { rxErrors++; }
}

uint32 Plat_Uart_GetRxDropped(void) { return rxDropped; }
uint32 Plat_Uart_GetRxErrors(void) { return rxErrors; }

Std_ReturnType Plat_Uart_Send(const char *text)
{
	static const uint8 lineEnd[] = {'\r', '\n'};
	uint32 length = 0U;

	if (text == NULL_PTR)
	{
	  return E_NOT_OK;
	}

	while (text[length] != '\0')
	{
		length++;
		if (UART_LIMIT_LEN < length) { break; }
	}

	/* Do not modify the caller's string; literals may reside in flash. */
	if ((length > 0U) &&
		(E_OK != Uart_SyncSend(0U, (const uint8 *)text, length, 100000U)))
	{
		return E_NOT_OK;
	}

	return Uart_SyncSend(0U, lineEnd, sizeof(lineEnd), 100000U);
}
#endif
