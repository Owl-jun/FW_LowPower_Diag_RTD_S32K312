#ifndef APP_CONFIG_CHECK_H
#define APP_CONFIG_CHECK_H

/* Included after the options in APP_CFG.h. These checks describe the
 * combinations supported by the current implementation, not MCU limits. */
#if ((UART_ENABLE != 0U) && (UART_ENABLE != 1U)) || \
    ((UART_LOG_ENABLE != 0U) && (UART_LOG_ENABLE != 1U)) || \
    ((TIMEBASE_ENABLE != 0U) && (TIMEBASE_ENABLE != 1U)) || \
    ((ADC_ENABLE != 0U) && (ADC_ENABLE != 1U)) || \
    ((DMA_ENABLE != 0U) && (DMA_ENABLE != 1U)) || \
    ((PWM_ENABLE != 0U) && (PWM_ENABLE != 1U)) || \
    ((WATCHDOG_ENABLE != 0U) && (WATCHDOG_ENABLE != 1U)) || \
    ((STANDBY_ENABLE != 0U) && (STANDBY_ENABLE != 1U)) || \
    ((_DEBUG_ADC_ != 0U) && (_DEBUG_ADC_ != 1U))
#error "Feature options must be 0 or 1"
#endif

#if (INIT_BOOT_MODE != BOOT_RUN)
#error "Boot must use RUN: request Standby after wakeup hardware initialization"
#endif

#if (UART_ENABLE != CFG_ENABLE)
#error "UART OFF is not supported yet: startup and generated callback still require it"
#endif
#if (TIMEBASE_ENABLE != CFG_ENABLE)
#error "Timebase OFF is not supported yet: generated GPT callback still requires it"
#endif
#if (ADC_ENABLE != CFG_ENABLE) || (DMA_ENABLE != CFG_ENABLE)
#error "ADC/DMA OFF is not supported yet: generated DMA callback still requires it"
#endif

#if (STANDBY_ENABLE == CFG_ENABLE) && (WATCHDOG_ENABLE == CFG_ENABLE)
#error "Current Standby entry does not support an enabled watchdog"
#endif
#if (STANDBY_ENABLE == CFG_ENABLE) && (PWM_ENABLE != CFG_ENABLE)
#error "Standby currently requires PWM initialization because it also initializes ICU"
#endif
#if (BUILD_MODE == BM_RELEASE) && (PWM_ENABLE != CFG_ENABLE)
#error "Release UART failure handling currently requires the PWM error LED"
#endif
#if (T_SEC_TIMEOUT_INF_LOOP == 0U)
#error "T_SEC_TIMEOUT_INF_LOOP must be greater than zero"
#endif

#endif
