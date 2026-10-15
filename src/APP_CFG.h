/* Application configuration. RTD hardware configuration remains in the .mex. */
#ifndef APP_CFG_H_
#define APP_CFG_H_

#include "APP_TYPE.h"

/* Select the default profile. May also be supplied by the build system. */
#ifndef BUILD_MODE
#define BUILD_MODE 				(BM_DEVELOPE)
#endif

/* Only settings that differ between profiles belong here. */
#if (BUILD_MODE == BM_DEVELOPE)
#define WATCHDOG_ENABLE 		(CFG_DISABLE)
#define UART_LOG_ENABLE 		(CFG_ENABLE)
#define _DEBUG_ADC_ 			(CFG_ON)
#elif (BUILD_MODE == BM_RELEASE)
#define WATCHDOG_ENABLE 		(CFG_ENABLE)
#define UART_LOG_ENABLE 		(CFG_DISABLE)
#define _DEBUG_ADC_ 			(CFG_OFF)
#else
#error "Unsupported BUILD_MODE: select BM_DEVELOPE or BM_RELEASE"
#endif

/* Shared settings: edit each option in one place. */
#define INIT_BOOT_MODE 				(BOOT_RUN)
#define UART_ENABLE 				(CFG_ENABLE)
#define TIMEBASE_ENABLE 			(CFG_ENABLE)
#define ADC_ENABLE 					(CFG_ENABLE)
#define PWM_ENABLE 					(CFG_ENABLE)
#define T_SEC_TIMEOUT_INF_LOOP 		(5U)	/* SEC */

/* The current ADC acquisition implementation always uses DMA. */
#define DMA_ENABLE 					(ADC_ENABLE)

/* Current Standby implementation rejects entry while watchdog is enabled.
 * Release therefore has Standby disabled explicitly instead of advertising
 * a feature that would be rejected at runtime. */
#define STANDBY_ENABLE 				(WATCHDOG_ENABLE == CFG_DISABLE)

#include "App_ConfigCheck.h"

#endif /* APP_CFG_H_ */
