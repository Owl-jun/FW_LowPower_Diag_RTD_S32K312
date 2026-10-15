#ifndef APP_TELEMETRY_H
#define APP_TELEMETRY_H

#include "../APP_CFG.h"

/* Main context: report boot time and ADC readings when the second changes.
 * Sends at most one report per call; missed seconds are not replayed. */
void App_Telemetry_Process(void);
#endif
