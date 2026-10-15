#ifndef APP_COMMAND_H
#define APP_COMMAND_H

/* Main context only. Consume at most 32 bytes and execute at most one
 * command per call. Partial commands are retained across calls. */
void App_Command_Process(void);

#endif
