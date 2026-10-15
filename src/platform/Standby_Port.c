#include "Standby_Port.h"
#include "../APP_CFG.h"




#include "S32K312_MC_ME.h"
#include "S32K312_MC_RGM.h"
#include "S32K312_WKPU.h"


volatile Plat_Standby_StageType Plat_Standby_Stage;
volatile Plat_Standby_BootInfoType Plat_Standby_BootInfo;

#define PLAT_STANDBY_WKPU_MASK (1UL << 3U)


#if (STANDBY_ENABLE == CFG_ENABLE)
#if (ADC_ENABLE == CFG_ENABLE)
#include "Adc_Port.h"
#endif
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
#define PLAT_STANDBY_CLOCK McuConf_McuClockSettingConfig_StandbyClock
#define PLAT_STANDBY_TIMER GptConf_GptChannelConfiguration_StandbyRti
#define PLAT_STANDBY_WAKE_CHANNEL IcuConf_IcuChannel_IcuChannel_1

static void Plat_Standby_Halt(Plat_Standby_StageType reason)
{
    Plat_Standby_Stage = reason;
    /* Peripherals/clocks may already be changed. Do not resume the RUN loop
     * or issue a software reset that could be mistaken for a successful wake. */
    for (;;) { /* Inspect Plat_Standby_Stage, then reset the board manually. */ }
}
#endif

void Plat_Standby_CaptureBootInfo(void)
{
    /* Ordinary .bss is initialized before main. Save hardware evidence before
     * Mcu_Init/Gpt_Init/Icu_Init can acknowledge or reconfigure it. */
    Plat_Standby_BootInfo.modeStat = IP_MC_ME->MODE_STAT;
    Plat_Standby_BootInfo.resetDes = IP_MC_RGM->DES;
    Plat_Standby_BootInfo.resetFes = IP_MC_RGM->FES;
    Plat_Standby_BootInfo.wkpuWisr = IP_WKPU->WISR;
    Plat_Standby_BootInfo.fromRti =
        ((Plat_Standby_BootInfo.wkpuWisr & PLAT_STANDBY_WKPU_MASK) != 0U) ? TRUE : FALSE;
    Plat_Standby_BootInfo.fromStandby =
        ((Plat_Standby_BootInfo.modeStat & MC_ME_MODE_STAT_PREV_MODE_MASK) ==
         MC_ME_MODE_STAT_PREV_MODE_MASK) ? TRUE : FALSE;

#if (STANDBY_ENABLE == CFG_ENABLE)
    /* WKPU belongs to the standby domain: acknowledge only our source,
     * after saving it. GPT initialization subsequently stops/reinitializes RTI. */
    IP_WKPU->WRER &= ~PLAT_STANDBY_WKPU_MASK;
    IP_WKPU->IRER &= ~PLAT_STANDBY_WKPU_MASK;
    IP_WKPU->WISR = PLAT_STANDBY_WKPU_MASK;
#endif
}

boolean Plat_Standby_IsAvailable(void)
{
#if (STANDBY_ENABLE == CFG_ENABLE)
    if ((PIT_IP_USED == CFG_ON) && (PIT_IP_RTI_USED == CFG_ON) &&
        (WKPU_IP_USED == CFG_ON) && (WATCHDOG_ENABLE == CFG_DISABLE))
    {
        return TRUE;
    }
#endif
    Plat_Standby_Stage = PLAT_STANDBY_DISABLED;
    return FALSE;
}

void Plat_Standby_Enter(void)
{
#if (STANDBY_ENABLE == CFG_ENABLE)
    uint32 rtiControl;

    if (!Plat_Standby_IsAvailable()) { return; }

    Plat_Standby_Stage = PLAT_STANDBY_PREPARING;

#if (TIMEBASE_ENABLE == CFG_ENABLE)
    Gpt_StopTimer(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);
    Gpt_DisableNotification(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);
#endif

    Icu_DisableEdgeDetection(IcuChannel_0);
    Icu_DisableNotification(IcuChannel_0);

#if (ADC_ENABLE == CFG_ENABLE)
    if (Plat_Adc_SetupResult == E_OK)
    {
        Adc_DisableHardwareTrigger(AdcGroup_0);
        if (Adc_GetGroupStatus(AdcGroup_0) != ADC_IDLE)
        {
            Plat_Standby_Halt(PLAT_STANDBY_QUIESCE_FAILED);
        }
    }
#endif

    /* Stop PWM outputs before the eMIOS clocks change. Mcl stays initialized
     * because it owns the shared DMA/trigger resources. */
    Pwm_DeInit();
    IntCtrl_Ip_DisableIrq(SIUL_1_IRQn);

#if (UART_ENABLE == CFG_ENABLE)
    IntCtrl_Ip_DisableIrq(LPUART6_IRQn);
    if (Uart_Abort(0U, UART_RECEIVE) != E_OK)
    {
        Plat_Standby_Halt(PLAT_STANDBY_QUIESCE_FAILED);
    }
#endif

#if (TIMEBASE_ENABLE == CFG_ENABLE)
    IntCtrl_Ip_DisableIrq(STM0_IRQn);
#endif

#if (DMA_ENABLE == CFG_ENABLE)
    IntCtrl_Ip_DisableIrq(DMATCD0_IRQn);
#endif

    IntCtrl_Ip_ClearPending(SIUL_1_IRQn);

#if (TIMEBASE_ENABLE == CFG_ENABLE)
    IntCtrl_Ip_ClearPending(STM0_IRQn);
#endif

#if (DMA_ENABLE == CFG_ENABLE)
    IntCtrl_Ip_ClearPending(DMATCD0_IRQn);
#endif

#if (UART_ENABLE == CFG_ENABLE)
    IntCtrl_Ip_ClearPending(LPUART6_IRQn);
#endif

    /* Generated FIRC configuration switches consumers away from PLL before
     * disabling it. Do not call Mcu_DistributePllClock for this configuration. */
    if (Mcu_InitClock(PLAT_STANDBY_CLOCK) != E_OK)
    {
        Plat_Standby_Halt(PLAT_STANDBY_CLOCK_FAILED);
    }
    Plat_Standby_Stage = PLAT_STANDBY_CLOCK_READY;

    /* Peripheral TIE/WRER must be enabled, but RUN ISRs must not consume the
     * wake event while the main core is entering Standby. */
    IntCtrl_Ip_DisableIrq(PIT0_IRQn);
    IntCtrl_Ip_DisableIrq(WKPU_IRQn);
    IntCtrl_Ip_ClearPending(PIT0_IRQn);
    IntCtrl_Ip_ClearPending(WKPU_IRQn);
    Icu_EnableEdgeDetection(PLAT_STANDBY_WAKE_CHANNEL);
    Gpt_EnableNotification(PLAT_STANDBY_TIMER);
    Gpt_StartTimer(PLAT_STANDBY_TIMER,
        PLAT_STANDBY_RTI_CLOCK_HZ * PLAT_STANDBY_DELAY_SECONDS);

    rtiControl = IP_PIT_0->RTI_TCTRL;
    if (((IP_WKPU->WRER & PLAT_STANDBY_WKPU_MASK) == 0U) ||
        ((rtiControl & (PIT_RTI_TCTRL_TEN_MASK | PIT_RTI_TCTRL_TIE_MASK)) !=
         (PIT_RTI_TCTRL_TEN_MASK | PIT_RTI_TCTRL_TIE_MASK)))
    {
        Plat_Standby_Halt(PLAT_STANDBY_ARM_FAILED);
    }
    Plat_Standby_Stage = PLAT_STANDBY_ARMED;
    MCAL_DATA_SYNC_BARRIER();
    Plat_Standby_Stage = PLAT_STANDBY_REQUESTED;
    Mcu_SetMode(McuModeSettingConf_1);

    /* Normal SoC Standby wake-up is expected through Reset_Handler/main.
     * WFI returning here is not sufficient proof of a Standby transition. */
    Gpt_StopTimer(PLAT_STANDBY_TIMER);
    Icu_DisableEdgeDetection(PLAT_STANDBY_WAKE_CHANNEL);
    Plat_Standby_Halt(PLAT_STANDBY_UNEXPECTED_RETURN);
#else
    Plat_Standby_Stage = PLAT_STANDBY_DISABLED;

#endif
}
