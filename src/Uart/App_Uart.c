/*
 * App_Uart.c
 *
 *  Created on: 2026. 9. 21.
 *      Author: David.Kang
 */

#include "App_Uart.h"

#include "../PWM/App_Pwm.h"
#include "../APP/App_Standby.h"

#include "Mcu.h"
#include "CDD_Uart.h"
#include "Lpuart_Uart_Ip.h"
#include "IntCtrl_Ip.h"
#include <stdlib.h>
#include "string.h"

#define RX_BUFFER_SIZE 	256U
#define RX_PACKET_SIZE 	8U
#define RX_PACKET_CNT  	2U
#define UART_LIMIT_LEN	4096U

typedef struct UART_PACKET
{
	char u8RxPacket[RX_PACKET_SIZE];
	uint8_t u8RxPtr;
}UART_PACKET_t;

void Reset_Packet(UART_PACKET_t * pPacket)
{
	memset(pPacket->u8RxPacket, '\0', RX_PACKET_SIZE);
	pPacket->u8RxPtr = 0U;
}

/* Single ISR producer, single main-loop consumer. Capacity: 255 bytes. */
static UART_PACKET_t rxPacket = {{0xA5U,} , 0U};
const char * u8Cmd[RX_PACKET_CNT] = {"950809" , "950808"};
static uint8 rxByte;
static volatile uint8 rxBuf[RX_BUFFER_SIZE];
static volatile uint32 rxHead;
static volatile uint32 rxTail;
static volatile uint32 rxDropped;
static volatile uint32 rxErrors;
static boolean rxStarted;

static volatile uint8   rxPacketNum = 0xFFU;
static volatile boolean rxPacketCompleted;



Std_ReturnType App_Uart_Recv_Async(void)
{
	Std_ReturnType result = Uart_AsyncReceive(0U, &rxByte, 1U);
	if (result == E_OK) { rxStarted = TRUE; }
	return result;
}

uint32 App_Uart_Read(uint8 *data, uint32 capacity)
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

void App_Uart_RxProcess(void)
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
	if (rxPacketCompleted)
	{
		if (rxPacketNum == 0U)
		{
			App_Uart_Send("Collect Command Key, Turn On Red LED");
			App_Pwm_On_RedLed(rxPacketNum);
		}
		else if (rxPacketNum == 1U)
		{
			App_Standby_Enter();
		}
		Reset_Packet(&rxPacket);
		rxPacketNum = 0xFFU;
		rxPacketCompleted = FALSE;
	}
}

void App_Uart_Callback(const uint8 channel, const Uart_EventType event)
{
	if (channel != 0U) { return; }
	if (event == UART_EVENT_RX_FULL)
	{
		uint32 next = (rxHead + 1U) % RX_BUFFER_SIZE;

		/* Keep a completed packet until the main loop executes it. */
		if (!rxPacketCompleted)
		{
			if (rxPacketNum == 0xFFU)
			{
				/* ASCII packet number: '0' + "950809", '1' + "950808". */
				if (rxByte == '0' || rxByte == '1') { rxPacketNum = rxByte - '0'; }
			}
			else if ((rxPacket.u8RxPtr < (RX_PACKET_SIZE - 1U)) &&
					 (rxByte == u8Cmd[rxPacketNum][rxPacket.u8RxPtr]))
			{
				rxPacket.u8RxPacket[rxPacket.u8RxPtr++] = rxByte;
				rxPacket.u8RxPacket[rxPacket.u8RxPtr] = '\0';

				if ((strcmp(rxPacket.u8RxPacket,u8Cmd[rxPacketNum]) == 0U))
				{
					rxPacketCompleted = TRUE;
				}
			}
			else
			{
				/* Clear Rx Packet */
				Reset_Packet(&rxPacket);
				rxPacketNum = 0xFFU;
			}
		}

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

uint32 App_Uart_GetRxDropped(void) { return rxDropped; }
uint32 App_Uart_GetRxErrors(void) { return rxErrors; }

Std_ReturnType App_Uart_Send(const char *text)
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
