/*
 * Platform_Init.c
 *
 *  Created on: 2026. 9. 17.
 *      Author: David.Kang
 */

#include "Platform_Init.h"
#include "../APP_CFG.h"
#include "Platform.h"
#include "Mcu.h"
#include "Mcl.h"
#include "Pwm.h"
#include "Icu.h"
#include "Gpt.h"
#include "Adc.h"
#include "Wdg_43_Instance0.h"
#include "CDD_Uart.h"
#include "CDD_Rm.h"
#include "Siul2_Port_Ip.h"
#include "Adc_Port.h"

void Plat_InitHardware(void)
{
	Adc_ValueGroupType * ptrDmaBuf = Plat_Adc_Get_Dma_Result_Buf();

	/* MCU in RTD Module INIT */
	Mcu_Init(NULL_PTR);
	uint8_t u8Ret = Mcu_InitClock(0);
	if (u8Ret != E_OK) { for (;;) { /* CLOCK Configure Failed */ } }

	/* Wait for PLL lock. */
	while (MCU_PLL_LOCKED != Mcu_GetPllStatus()) { /* DUMMY */ }

	/* Switch PLL-fed clock muxes to their configured sources. */
	if (E_OK != Mcu_DistributePllClock()) { for (;;) { /* PLL distribution failed */ } }

#if (INIT_BOOT_MODE == BOOT_RUN)
	/* MCU Set Mode 0 (RUN) */
	Mcu_SetMode(0U);
#elif (INIT_BOOT_MODE == BOOT_STANDBY)
	/* MCU Set Mode 1 (STANDBY)
	 * If selected boot mode 1, CPU State is to wait for interrupt(WFI),
	 * not execute below SW codes and wake-up by RTI request (wake-up after about 10-second)
	 * */
	Mcu_SetMode(1U);
#endif


#if (WATCHDOG_ENABLE == CFG_ENABLE)
/* WATCHDOG */
	Wdg_43_Instance0_Init(NULL_PTR);
#endif

#if (TIMEBASE_ENABLE == CFG_ENABLE)
	/* STM0 in GPT Timer */
	Gpt_Init(NULL_PTR);
	Gpt_EnableNotification(0U);
#endif

	/* PORT */
	Siul2_Port_Ip_Init(	NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals,
						g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals );

	/* NVIC ... */
	Platform_Init(NULL_PTR);

	/* Create Clock BUS */
	Mcl_Init(NULL_PTR);

	/* PWM */
	Pwm_Init(NULL_PTR);

	/* ICU IRQ ENABLE */
	Icu_Init(NULL_PTR);
	Icu_EnableNotification(IcuChannel_0);
	Icu_EnableEdgeDetection(IcuChannel_0);

#if (UART_ENABLE == CFG_ENABLE)
	/* UART */
	Uart_Init(NULL_PTR);
#endif

#if (ADC_ENABLE == CFG_ENABLE && DMA_ENABLE == CFG_ENABLE)
	MCAL_DATA_SYNC_BARRIER();
	/* Resource Manager */
	Rm_Init(NULL_PTR);
	MCAL_DATA_SYNC_BARRIER();

	/* ADC */
	Adc_Init(NULL_PTR);
	MCAL_DATA_SYNC_BARRIER();
	ptrDmaBuf[0] = 0xFFFFU; /* Sentinel: no ADC result received yet. */
	Plat_Adc_SetupResult = Adc_SetupResultBuffer(AdcGroup_0, ptrDmaBuf);
	if (Plat_Adc_SetupResult == E_OK)
	{
		Adc_EnableHardwareTrigger(0U);
	}
#endif

#if (TIMEBASE_ENABLE == CFG_ENABLE)
	/* Clock Source : 48MHz, 4800000tick is 100ms */
	Gpt_StartTimer(0U, 4800000U);
#endif
}
