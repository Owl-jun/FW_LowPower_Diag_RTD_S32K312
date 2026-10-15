/*
 * main.c
 *
 *  Created on: 2026. 9. 17.
 *      Author: David.Kang
 */

#include "APP/App.h"


/* MAIN */
int main(void)
{
    App_Init();

    for(;;)	/* Main Loop */
    {
        App_Process();
    }

    return 0U;
}

