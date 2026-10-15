/*
 * Adc_Port.c
 *
 *  Created on: 2026. 9. 30.
 *      Author: David.Kang
 */
#include "Adc_Port.h"
#if (ADC_ENABLE == CFG_ENABLE && DMA_ENABLE == CFG_ENABLE)
#include "Adc.h"

/* RTD DMA completion entry point, normally selected as the MCL callback. */
extern void Adc_Ipw_Adc0DmaTransferCompleteNotification(void);

static Adc_ValueGroupType aDmaBuf[ADC_DMA_RESULT_BUF_SIZE] __attribute__((section(".mcal_bss_no_cacheable"), aligned(32)));
volatile Std_ReturnType Plat_Adc_SetupResult = E_NOT_OK;

#if (_DEBUG_ADC_ == CFG_ON)
volatile uint32 Plat_Adc_DmaCompleteCount = 0U;
volatile Adc_ValueGroupType Plat_Adc_LastResult = 0xFFFFU;
volatile Adc_StatusType Plat_Adc_LastStatus = ADC_IDLE;
#endif

Adc_ValueGroupType * Plat_Adc_Get_Dma_Result_Buf(void)
{
	return aDmaBuf;
}

Adc_ValueGroupType Plat_Adc_ReadRaw(void)
{
    /* DMA writes this non-cacheable storage asynchronously. */
    return *((volatile Adc_ValueGroupType *)&aDmaBuf[0]);
}

void Plat_Adc_DmaCallback(void)
{
    /* Required: update ADC state and prepare DMA for the next HW trigger. */
    Adc_Ipw_Adc0DmaTransferCompleteNotification();

#if (_DEBUG_ADC_ == CFG_ON)
    Plat_Adc_LastResult = aDmaBuf[0];
    Plat_Adc_LastStatus = Adc_GetGroupStatus(AdcGroup_0);
    Plat_Adc_DmaCompleteCount++;
#endif
}
#endif
