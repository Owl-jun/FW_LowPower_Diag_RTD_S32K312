#ifndef APP_H
#define APP_H

#include "../APP_CFG.h"




/* Call once at boot, before any peripheral initialization. Captures the
 * wakeup/reset evidence, initializes hardware, and starts UART reception.
 * Returns only when initialization succeeds. */
void App_Init(void);

/* Call repeatedly in main context after App_Init. Runs one iteration of
 * application work. A Standby command can leave through the reset path. */
void App_Process(void);

#endif
