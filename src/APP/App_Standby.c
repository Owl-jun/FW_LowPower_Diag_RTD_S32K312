#include "App_Standby.h"
#include "../Uart/App_Uart.h"
#include "S32K312_MC_ME.h"
#include "S32K312_MC_RGM.h"
#include "S32K312_WKPU.h"
#include <stdio.h>
#include "../APP_CFG.h"

volatile App_Standby_StageType App_Standby_Stage;
volatile App_Standby_BootInfoType App_Standby_BootInfo;

#define APP_STANDBY_WKPU_MASK (1UL << 3U)

#if (STANDBY_ENABLE == CFG_ENABLE)
#include "../INIT/init.h"
#include "../ADC/App_Adc.h"
#include "Mcu.h"
#include "Gpt.h"
#include "Icu.h"
#include "Adc.h"
#include "Pwm.h"
#include "CDD_Uart.h"
#include "IntCtrl_Ip.h"
#include "Pit_Ip_Cfg_Defines.h"
#include "Wkpu_Ip_Defines.h"
#include "S32K312_PIT.h"

/* Generated logical channel names; IcuChannel_1 maps to WKPU source 3. */
#define APP_STANDBY_CLOCK McuConf_McuClockSettingConfig_StandbyClock
#define APP_STANDBY_TIMER GptConf_GptChannelConfiguration_StandbyRti
#define APP_STANDBY_WAKE_CHANNEL IcuConf_IcuChannel_IcuChannel_1

static void App_Standby_Halt(App_Standby_StageType reason)
{
    App_Standby_Stage = reason;
    /* Peripherals/clocks may already be changed. Do not resume the RUN loop
     * or issue a software reset that could be mistaken for a successful wake. */
    for (;;) { /* Inspect App_Standby_Stage, then reset the board manually. */ }
}
#endif

void App_Standby_CaptureBootInfo(void)
{
    /* Ordinary .bss is initialized before main. Save hardware evidence before
     * Mcu_Init/Gpt_Init/Icu_Init can acknowledge or reconfigure it. */
    App_Standby_BootInfo.modeStat = IP_MC_ME->MODE_STAT;
    App_Standby_BootInfo.resetDes = IP_MC_RGM->DES;
    App_Standby_BootInfo.resetFes = IP_MC_RGM->FES;
    App_Standby_BootInfo.wkpuWisr = IP_WKPU->WISR;
    App_Standby_BootInfo.fromStandby =
        ((App_Standby_BootInfo.modeStat & MC_ME_MODE_STAT_PREV_MODE_MASK) ==
         MC_ME_MODE_STAT_PREV_MODE_MASK) ? TRUE : FALSE;

#if (STANDBY_ENABLE == CFG_ENABLE)
    /* WKPU belongs to the standby domain: acknowledge only our source,
     * after saving it. GPT initialization subsequently stops/reinitializes RTI. */
    IP_WKPU->WRER &= ~APP_STANDBY_WKPU_MASK;
    IP_WKPU->IRER &= ~APP_STANDBY_WKPU_MASK;
    IP_WKPU->WISR = APP_STANDBY_WKPU_MASK;
#endif
}

void App_Standby_ReportBootInfo(void)
{
    char text[160];
    (void)snprintf(text, sizeof(text),
        "Boot: standby=%u RTI-source=%u MODE=%08lx DES=%08lx FES=%08lx WISR=%08lx",
        (unsigned int)App_Standby_BootInfo.fromStandby,
        (unsigned int)((App_Standby_BootInfo.wkpuWisr & APP_STANDBY_WKPU_MASK) != 0U),
        (unsigned long)App_Standby_BootInfo.modeStat,
        (unsigned long)App_Standby_BootInfo.resetDes,
        (unsigned long)App_Standby_BootInfo.resetFes,
        (unsigned long)App_Standby_BootInfo.wkpuWisr);
    (void)App_Uart_Send(text);
}

void App_Standby_RtiNotification(void)
{
    /* This test keeps PIT0's NVIC IRQ disabled. The peripheral's TIE remains
     * enabled to drive WKPU. No application work is performed in an ISR. */
}

void App_Standby_Enter(void)
{
#if (STANDBY_ENABLE == CFG_ENABLE)
    uint32 rtiControl;

    if ((PIT_IP_USED != CFG_ON) || (PIT_IP_RTI_USED != CFG_ON) ||
        (WKPU_IP_USED != CFG_ON) || (WATCHDOG_ENABLE != CFG_DISABLE))
    {
        App_Standby_Stage = APP_STANDBY_DISABLED;
        (void)App_Uart_Send("Standby blocked: check RTD configuration / watchdog.");
        return;
    }
    if (App_Uart_Send("Entering Standby; PIT0 RTI wake-up in about 3 seconds.") != E_OK)
    {
        return;
    }

    App_Standby_Stage = APP_STANDBY_PREPARING;
    Gpt_StopTimer(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);
    Gpt_DisableNotification(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);
    Icu_DisableEdgeDetection(IcuChannel_0);
    Icu_DisableNotification(IcuChannel_0);
    if (App_Adc_SetupResult == E_OK)
    {
        Adc_DisableHardwareTrigger(AdcGroup_0);
        if (Adc_GetGroupStatus(AdcGroup_0) != ADC_IDLE)
        {
            App_Standby_Halt(APP_STANDBY_QUIESCE_FAILED);
        }
    }
    /* Stop PWM outputs before the eMIOS clocks change. Mcl stays initialized
     * because it owns the shared DMA/trigger resources. */
    Pwm_DeInit();
    IntCtrl_Ip_DisableIrq(LPUART6_IRQn);
    if (Uart_Abort(0U, UART_RECEIVE) != E_OK)
    {
        App_Standby_Halt(APP_STANDBY_QUIESCE_FAILED);
    }
    IntCtrl_Ip_DisableIrq(STM0_IRQn);
    IntCtrl_Ip_DisableIrq(SIUL_1_IRQn);
    IntCtrl_Ip_DisableIrq(DMATCD0_IRQn);
    IntCtrl_Ip_ClearPending(STM0_IRQn);
    IntCtrl_Ip_ClearPending(SIUL_1_IRQn);
    IntCtrl_Ip_ClearPending(DMATCD0_IRQn);
    IntCtrl_Ip_ClearPending(LPUART6_IRQn);

    /* Generated FIRC configuration switches consumers away from PLL before
     * disabling it. Do not call Mcu_DistributePllClock for this configuration. */
    if (Mcu_InitClock(APP_STANDBY_CLOCK) != E_OK)
    {
        App_Standby_Halt(APP_STANDBY_CLOCK_FAILED);
    }
    App_Standby_Stage = APP_STANDBY_CLOCK_READY;

    /* Peripheral TIE/WRER must be enabled, but RUN ISRs must not consume the
     * wake event while the main core is entering Standby. */
    IntCtrl_Ip_DisableIrq(PIT0_IRQn);
    IntCtrl_Ip_DisableIrq(WKPU_IRQn);
    IntCtrl_Ip_ClearPending(PIT0_IRQn);
    IntCtrl_Ip_ClearPending(WKPU_IRQn);
    Icu_EnableEdgeDetection(APP_STANDBY_WAKE_CHANNEL);
    Gpt_EnableNotification(APP_STANDBY_TIMER);
    Gpt_StartTimer(APP_STANDBY_TIMER,
        APP_STANDBY_RTI_CLOCK_HZ * APP_STANDBY_DELAY_SECONDS);

    rtiControl = IP_PIT_0->RTI_TCTRL;
    if (((IP_WKPU->WRER & APP_STANDBY_WKPU_MASK) == 0U) ||
        ((rtiControl & (PIT_RTI_TCTRL_TEN_MASK | PIT_RTI_TCTRL_TIE_MASK)) !=
         (PIT_RTI_TCTRL_TEN_MASK | PIT_RTI_TCTRL_TIE_MASK)))
    {
        App_Standby_Halt(APP_STANDBY_ARM_FAILED);
    }
    App_Standby_Stage = APP_STANDBY_ARMED;
    MCAL_DATA_SYNC_BARRIER();
    App_Standby_Stage = APP_STANDBY_REQUESTED;
    Mcu_SetMode(McuModeSettingConf_1);

    /* Normal SoC Standby wake-up is expected through Reset_Handler/main.
     * WFI returning here is not sufficient proof of a Standby transition. */
    Gpt_StopTimer(APP_STANDBY_TIMER);
    Icu_DisableEdgeDetection(APP_STANDBY_WAKE_CHANNEL);
    App_Standby_Halt(APP_STANDBY_UNEXPECTED_RETURN);
#else
    App_Standby_Stage = APP_STANDBY_DISABLED;
    (void)App_Uart_Send("Standby disabled: complete docs/standby-rti.md, then enable APP_STANDBY_ENABLE.");
#endif
}
