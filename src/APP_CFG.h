/*
 * APP_CFG.h
 *
 *  Created on: 2026. 10. 15.
 *      Author: David.Kang
 */

#ifndef APP_CFG_H_
#define APP_CFG_H_

#include "APP_TYPE.h"

/* SELECT BOOT MODE AT INITIALIZE TIMING
 * BOOT_RUN 	: RUN MODE
 * BOOT_STANDBY : STANDBY MODE */
#define INIT_BOOT_MODE					(BOOT_RUN)

/* BUILD MODE SELECT
 * BM_DEVELOPE : DEBUG
 * BM_RELEASE : RELEASE */
#define BUILD_MODE						(BM_DEVELOPE)

/* MODULE ENABLE/DISABLE */
#if (BUILD_MODE == BM_DEVELOPE)
	#define WATCHDOG_ENABLE 				(CFG_DISABLE)
	#define STANDBY_ENABLE 					(CFG_ENABLE)
#elif (BUILD_MODE == BM_RELEASE)
	#define WATCHDOG_ENABLE 				(CFG_ENABLE)
	#define STANDBY_ENABLE 					(CFG_ENABLE)
#else
#endif

/* TIMEOUT CFGs (SEC) */
#define T_SEC_TIMEOUT_INF_LOOP 			(5U)	/* 5 Seconds */

#endif /* APP_CFG_H_ */
