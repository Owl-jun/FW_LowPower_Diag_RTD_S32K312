/*
 * APP_CFG.h
 *
 *  Created on: 2026. 10. 15.
 *      Author: David.Kang
 */

#ifndef APP_CFG_H_
#define APP_CFG_H_

#include "APP_TYPE.h"

/* BUILD MODE SELECT
 * BM_DEVELOPE : DEBUG
 * BM_RELEASE : RELEASE */
#define BUILD_MODE						(BM_DEVELOPE)

/* USE MODULE SET FOR BUILD_MODE */
#if (BUILD_MODE == BM_DEVELOPE)
	#define INIT_BOOT_MODE					(BOOT_RUN)
	#define WATCHDOG_ENABLE 				(CFG_DISABLE)
	#define	UART_LOG_ENABLE					(CFG_ENABLE)
	#define	UART_ENABLE						(CFG_ENABLE)	/* It Must be CFG_ENABLE If use UART_LOG_ENABLE option */
	#define TIMEBASE_ENABLE					(CFG_ENABLE)	/* It Must be CFG_ENABLE If use UART_LOG_ENABLE option */
	#define STANDBY_ENABLE 					(CFG_ENABLE)
	#define	ADC_ENABLE						(CFG_ENABLE)	/* It Must be CFG_ENABLE If use DMA_ENABLE option */
	#define _DEBUG_ADC_						(CFG_ON)
	#define	DMA_ENABLE						(CFG_ENABLE)	/* It Must be CFG_ENABLE If use ADC_ENABLE option */
#elif (BUILD_MODE == BM_RELEASE)
	#define INIT_BOOT_MODE					(BOOT_RUN)
	#define WATCHDOG_ENABLE 				(CFG_ENABLE)
	#define	UART_LOG_ENABLE					(CFG_DISABLE)
	#define	UART_ENABLE						(CFG_ENABLE)	/* It Must be CFG_ENABLE If use UART_LOG_ENABLE option */
	#define TIMEBASE_ENABLE					(CFG_ENABLE)	/* It Must be CFG_ENABLE If use UART_LOG_ENABLE option */
	#define STANDBY_ENABLE 					(CFG_ENABLE)
	#define	ADC_ENABLE						(CFG_ENABLE)	/* It Must be CFG_ENABLE If use DMA_ENABLE option */
	#define _DEBUG_ADC_						(CFG_OFF)
	#define	DMA_ENABLE						(CFG_ENABLE)	/* It Must be CFG_ENABLE If use ADC_ENABLE option */
#else
#endif

/* TIMEOUT CFGs (SEC) */
#define T_SEC_TIMEOUT_INF_LOOP 			(5U)	/* 5 Seconds */

#endif /* APP_CFG_H_ */
