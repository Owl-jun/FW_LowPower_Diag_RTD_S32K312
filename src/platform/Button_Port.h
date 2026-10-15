#ifndef PLATFORM_BUTTON_PORT_H
#define PLATFORM_BUTTON_PORT_H

#include "Std_Types.h"

/* Raw PTB19 input, normalized to TRUE for high. No debounce or edge detection. */
boolean Plat_Button_IsPwmInputHigh(void);

#endif
