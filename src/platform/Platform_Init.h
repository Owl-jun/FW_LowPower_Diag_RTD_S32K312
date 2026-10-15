/*
 * Platform_Init.h
 *
 *  Created on: 2026. 9. 17.
 *      Author: David.Kang
 */

#ifndef PLATFORM_INIT_H
#define PLATFORM_INIT_H

/* Call once per boot, after capturing reset/wakeup status.
 * Configures RTD peripherals and starts the existing ADC/timebase services.
 * Distinct from NXP's Platform_Init(), which initializes the Platform driver. */
void Platform_InitHardware(void);

#endif
