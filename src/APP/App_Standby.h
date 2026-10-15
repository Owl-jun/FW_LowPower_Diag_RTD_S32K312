#ifndef APP_STANDBY_H_
#define APP_STANDBY_H_

/* Called by App_Init before peripheral initialization. */
void App_Standby_CaptureBootInfo(void);
void App_Standby_ReportBootInfo(void);
/* Main context: check availability, log the request, then enter Standby. */
void App_Standby_Enter(void);
/* Existing RTD GPT callback name retained for regeneration compatibility. */
void App_Standby_RtiNotification(void);

#endif
