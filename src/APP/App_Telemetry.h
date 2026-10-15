#ifndef APP_TELEMETRY_H
#define APP_TELEMETRY_H

#include "../APP_CFG.h"

#if (UART_ENABLE == CFG_ENABLE			\
	&& UART_LOG_ENABLE == CFG_ENABLE	\
	&& TIMEBASE_ENABLE == CFG_ENABLE)
/* Main context: report boot time and ADC readings when the second changes.
 * Sends at most one report per call; missed seconds are not replayed. */
void App_Telemetry_Process(void);
#endif

#endif
