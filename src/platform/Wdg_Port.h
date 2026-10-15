/*
 * Wdg_Port.h
 *
 *  Created on: 2026. 9. 22.
 *      Author: David.Kang
 */

#ifndef PLATFORM_WDG_PORT_H
#define PLATFORM_WDG_PORT_H

/* Controlled by WATCHDOG_ENABLE. OFF makes these calls no-ops; it does
 * not disable a hardware watchdog already enabled by boot code. */
void Plat_Wdg_Init(void);
void Plat_Wdg_Refresh(void);
/* Always defined for the generated RTD callback reference.
 * ON: optional log, service once, then halt. OFF: return immediately. */
void Plat_Wdg_Callback(void);

#endif
