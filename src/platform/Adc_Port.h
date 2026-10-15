/*
 * Adc_Port.h
 *
 *  Created on: 2026. 9. 30.
 *      Author: David.Kang
 */

#ifndef PLATFORM_ADC_PORT_H
#define PLATFORM_ADC_PORT_H

#include "../APP_CFG.h"

#if (ADC_ENABLE == CFG_ENABLE && DMA_ENABLE == CFG_ENABLE)

#include "Adc_Types.h"

#define ADC_DMA_RESULT_BUF_SIZE			(1U)

Adc_ValueGroupType * Plat_Adc_Get_Dma_Result_Buf(void);
/* Latest raw DMA sample; may be the initialization sentinel before the
 * first conversion. Does not start a conversion or wait for completion. */
Adc_ValueGroupType Plat_Adc_ReadRaw(void);
void Plat_Adc_DmaCallback(void);

/* Live debugger watch values; completion count is updated last by the ISR. */
#if (_DEBUG_ADC_ == CFG_ON)
extern volatile uint32 Plat_Adc_DmaCompleteCount;
extern volatile Adc_ValueGroupType Plat_Adc_LastResult;
extern volatile Adc_StatusType Plat_Adc_LastStatus;
#endif
extern volatile Std_ReturnType Plat_Adc_SetupResult;

#endif

#endif /* PLATFORM_ADC_PORT_H */
