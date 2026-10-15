# S32K312: UART 명령으로 Standby 진입 후 PIT0 RTI 자동 복귀

## 동작과 범위

`1950808`을 UART로 보내면 약 3초 Standby 테스트를 요청한다. 기존 `0950809`의 빨간 LED 동작은 유지한다.
일반 `POWER_IP_STANDBY_MODE`를 사용하며 Fast Standby/별도 resume vector는 사용하지 않는다.
정상 복귀 경로는 `Reset_Handler -> main -> ALL_COMPONENT_INIT()`이다.
일반 .data/.bss와 드라이버 상태는 startup에서 초기화되고, 기존 MCU/클럭/GPT/ICU/UART/ADC/PWM 초기화가 다시 실행된다.
Standby RAM, WKPU 등 유지되는 영역/레지스터까지 모두 지워진다는 의미는 아니다.

코드는 `main()` 첫 줄에서 MC_ME.MODE_STAT, MC_RGM.DES/FES, WKPU.WISR를 저장한다.
RTD 초기화 이후 UART로 저장된 값을 출력한다. WISR만으로 성공 판정하지 말고 이전 모드도 함께 확인한다.
`App_Standby_BootInfo`는 현재 부팅에서 읽은 기록이며, 부팅 간 누적 카운터/보존 RAM이 아니다.

RTD 생성 파일과 .mex는 이 변경에서 수정하지 않았다. 아래 설정을 생성하고 검토한 뒤
`src/APP/App_Standby.h`의 `APP_STANDBY_ENABLE`을 `STD_ON`으로 바꾼다.
그 전에는 UART 1번 명령이 안내만 출력하며 Standby에 들어가지 않는다.

## 1. MCU/Clocks

- 기존 RUN clock ID 0과 mode ID 0은 유지한다.
- S32DS **Clocks Tool**에서 기존 `ClockConfig0` 옆의 설정 관리 기능으로 새 clock configuration을
  추가/복제한다. 이름은 **StandbyClock**으로 한다. MCU 항목부터 수동 추가하지 않는다.
  이후 Peripherals > Mcu > McuModuleConfiguration > McuClockSettingConfig에서
  동기화된 설정과 ID **1**을 확인한다. 생성되는 MCU 설정 이름은 도구의 매핑에 따라 다를 수 있으므로
  `McuConf_McuClockSettingConfig_StandbyClock`과 다르면 App_Standby.c의 APP_STANDBY_CLOCK을 실제 심벌로 맞춘다.
  Power mode ID와 clock ID는 서로 다른 설정이다.
- 진입용 설정: FIRC 활성화, CORE_CLK의 소스를 FIRC로 지정한다(예: 48 MHz).
  다른 PLL 의존 mux도 FIRC 등 유효한 소스로 변경하고 PLL은 비활성화한다.
  AIPS/버스 분주 등은 Config Tools 검증을 통과하는 값으로 설정한다.
- SIRC(공칭 32 kHz)를 활성화하고 **Standby에서도 SIRC 유지**를 활성화한다.
  RTI 입력은 SIRC 32 kHz 기준이다. PIT0 일반 채널의 버스 클럭과 혼동하지 않는다.
- 초기 검증에서는 주변장치 gate를 대량으로 제거하지 말고 기존 gate를 유지한다.
  특히 PIT0/WKPU와 RTD timeout에 필요한 클럭을 유지한다.
- MCU mode ID 1은 `POWER_IP_STANDBY_MODE`, Main Core CM7_0, CM7_0 Under MCU Control 유지,
  Sleep On Exit는 FALSE로 설정한다.
- mode 1 COFB 설정에서 PIT0(Partition 0 COFB1 REQ44), WKPU(Partition 1 COFB1 REQ45)를 유지한다.
  현재 생성 설정에는 두 비트 모두 포함되어 있다.
- 생성 후 `McuConf_McuClockSettingConfig_StandbyClock` 심벌을 확인한다.

코드는 `Mcu_InitClock(StandbyClock)`을 호출한다. RTD의 안전한 clock 전환 경로를 사용하며
PLL 레지스터를 직접 끄지 않는다. 이 설정에서는 `Mcu_DistributePllClock()`을 호출하지 않는다.

## 2. GPT: 기존 STM0 유지 + PIT0 RTI 추가

- 기존 `GptChannelConfiguration_0`(logical ID 0 / STM0)은 유지한다.
- GPT의 PIT 하드웨어 설정에 **PIT_0** 인스턴스를 추가한다.
- 그 인스턴스에 **RTI 전용 채널**을 추가한다. 일반 CH_0/CH_1과 다르다.
  UI에서 RTI를 선택했는지는 생성된 PIT 채널 설정의 `RTI` 선택으로 재확인한다.
- GPT logical channel 이름 **StandbyRti**, ID **1**을 추가하고 위 PIT0 RTI에 연결한다.
- Mode: ONE SHOT, Tick Value Max: 4294967295, RTI clock reference: SIRC 32000 Hz.
- Notification: **App_Standby_RtiNotification**.
- Enable/Disable Notification API를 활성화한다(현재 이미 ON).
- 초기 시험에서는 Freeze in debug를 OFF로 두고 타이머 시작 이후는 Resume으로 실행한다.
- 생성 결과: `PIT_IP_USED=STD_ON`, `PIT_IP_RTI_USED=STD_ON`,
  `GptConf_GptChannelConfiguration_StandbyRti` 심벌 존재를 확인한다.

이 bare-metal 테스트는 `Gpt_SetMode(GPT_MODE_SLEEP)` / EcuM 경로를 사용하지 않는다.
GPT Wakeup Functionality/Report Wakeup Source/EcuM 설정은 이 코드 때문에 추가할 필요가 없다.
`Gpt_EnableNotification()`으로 RTI의 TIE를 켜고, WKPU를 별도로 준비한다.
타이머는 32000 * 3 = **96000 ticks**로 시작하며 RTD가 LDVAL에 count-1을 기록한다.
정확한 3초가 아니라 SIRC 허용오차를 포함한 대략 3초다.

## 3. ICU: WKPU 내부 source 3 추가

- **먼저 기존 별도 `Wkpu` IP 컴포넌트를 Peripherals 구성에서 제거한다.**
  이 프로젝트에는 NMI 설정을 가진 별도 `Wkpu`(instance 0) 컴포넌트가 이미 존재했다.
  NMI request가 FALSE여도 인스턴스 설정 소유권이 없어지는 것은 아니다.
  이것을 둔 채 `Icu`에도 WKPU instance 0을 추가하면
  `A peripheral has been allocated/configured more than once` 오류가 발생할 수 있다.
  RTD 소스 파일을 수동으로 삭제하지 말고 컴포넌트 구성에서 소유자를 ICU 하나로 만든다.
- 기존 SIUL2 버튼 채널 `IcuChannel_0`은 유지한다.
- ICU의 WKPU 하드웨어 설정에 **WKPU source/channel 3 (PIT0 RTI)**을 추가한다.
- Rising edge, filter OFF로 설정한다. 외부 WKPU 핀/핀 mux 추가는 필요 없다.
- ICU logical channel 이름 **IcuChannel_1**, ID **1**을 추가한다.
  Measurement Mode는 SIGNAL_EDGE_DETECT, 위 WKPU3 하드웨어 채널에 연결한다.
- ICU의 **IcuHwInterruptConfigList**에도 WKPU 인스턴스 0 / 채널 3에 해당하는
  `IcuIsrHwId` 항목을 추가한다. RTD 7.0.1 검증은 logical channel 연결뿐 아니라
  이 목록의 등록도 요구한다. 누락되면 채널 번호 칸에
  `No logic channel use this WKPU channel.` 또는
  `ISR for this channel is not configured in HwInterruptConfigList` 오류가 뜰 수 있다.
  이 등록은 Platform/NVIC IRQ enable과는 별개이므로 아래 NVIC disable 방침은 유지한다.
- 본 코드가 사용하는 것은 **Icu(MCAL) 컴포넌트**다. 별도 **Wkpu(IP) 컴포넌트**에만
  채널을 추가하면 `Icu_Init()` 및 `IcuConf_IcuChannel_IcuChannel_1`에 연결되지 않는다.
  같은 WKPU3을 두 컴포넌트에서 중복 소유하지 않도록 한다.
- 생성 결과: `WKPU_IP_USED=STD_ON`,
  `IcuConf_IcuChannel_IcuChannel_1` 심벌 존재를 확인한다.
- 기존 `Icu_Init(NULL_PTR)`가 이 채널까지 초기화한다. 별도 `Wkpu_Ip_Init()` 중복 호출은 하지 않는다.

코드의 `Icu_EnableEdgeDetection()`은 이 RTD에서 `Wkpu_Ip_EnableInterrupt()`로 연결되어
WKPU IRER와 WRER를 함께 켠다. 이 테스트에는 `Icu_SetMode`/`Icu_EnableWakeup`/EcuM 경로가 필요하지 않다.
Standby Wakeup Support 옵션은 초기화 시 보존 플래그를 callback으로 처리하는 기능이다.
여기서는 초기화 전에 직접 기록하므로 초기 시험에서는 기존 OFF를 유지한다.

## 4. NVIC와 기존 주변장치

PIT0와 WKPU의 **NVIC IRQ는 이 테스트에서 활성화하지 않는다**.
코드에서도 arm 직전에 두 IRQ를 disable/clear한다. Peripheral RTI TIE와 WKPU WRER는 활성화한다.
WKPU 하드웨어의 wake-up 요청과 CPU가 RUN 중 ISR을 처리하는 것은 별개다.
이렇게 해서 진입 중 또는 부팅 초기 ISR이 원인 플래그를 먼저 지우는 것을 피한다.
RUN에서 RTI ISR 자체를 시험하려면 별도의 handler 등록/enable 시험으로 진행한다.

기존 `PWM_DE_INIT_API=STD_ON`, ADC hardware trigger API, GPT notification API,
ICU edge-detection API는 유지한다. Watchdog는 현재와 같이 OFF로 검증한다.

진입 함수는 UART 전송 완료, STM 정지, SIUL2 버튼 감지 중지, ADC trigger 중지 및 idle 확인,
PWM DeInit, UART RX abort 후 클럭을 바꾼다. 활성 IRQ의 일부도 disable/clear한다.
추후 다른 DMA/주변장치를 추가하면 해당 작업 정리도 진입 함수에 추가해야 한다.

## 5. 생성 및 실행

1. Config Tools validation 후 Generate Code.
2. 위 세 생성 심벌과 PIT/WKPU 활성화 매크로를 확인한다.
3. `APP_STANDBY_ENABLE=STD_ON`으로 변경한다.
4. 프로젝트 Refresh 후 Build한다. S32DS가 `src/APP/App_Standby.c`를 소스 목록에 추가했는지 확인한다.
5. Flash/실행 후 UART 부팅 로그 확인. 자동으로 Standby에 들어가지는 않는다.
6. UART `1950808` 전송. 진입 로그 이후 약 3초 뒤 새 부팅 로그를 기대한다.
7. 성공 근거: `standby=1`, `RTI-source=1`, 그리고 저장된 raw reset/wakeup 정보가 시험과 일치.
   보통의 리셋/전원 재인가로 다시 main에 왔다는 사실만으로 성공 처리하지 않는다.

Debug watch:

| 항목 | 의미 |
|---|---|
| `App_Standby_Stage` | 진입 진행 단계 / 실패 이유 |
| `App_Standby_BootInfo` | RTD 초기화 전 부팅 원인 스냅샷 |
| `IP_PIT_0->RTI_TCTRL` | arm 후 TEN=1, TIE=1 |
| `IP_PIT_0->RTI_CVAL` | 타이머 카운트 진행 확인 |
| `IP_WKPU->WRER & 8` | source 3 wake-up enable |
| `IP_WKPU->WIREER & 8` | source 3 rising-edge enable |

`APP_STANDBY_REQUESTED`는 요청 직전이며 실제 진입 완료 표시가 아니다.
`CLOCK_FAILED`, `ARM_FAILED`, `QUIESCE_FAILED`, `UNEXPECTED_RETURN`이면
부분적으로 변경된 주변장치 상태로 RUN 루프를 계속하지 않고 해당 상태로 정지한다.
디버거로 상태를 확인한 후 수동 리셋한다. 자동 software reset을 걸어 성공처럼 보이게 하지 않는다.

WFI 직전까지 BP로 확인한 후 Resume한다. RTI 시작 이후 오래 BP에서 멈추면
타이머가 먼저 만료되어 즉시 복귀할 수 있다. Standby 중 디버거 연결이 끊기는 것만으로 성공 판정하지 않는다.
SW 검증은 wake-up 경로/상태 검증이며 실제 소비전류는 별도 측정 대상이다.

## 근거

- 프로젝트 RTD 7.0.1: `Power_Ip_SetMode`, `Power_Ip_MC_ME_SocStandbyEntry`,
  `Wkpu_Ip_EnableInterrupt`, `Gpt_Ipw_EnableInterrupt`, `Pit_Ip_StartChannel`.
- [NXP RTI Cycle Wake-up 예제](https://community.nxp.com/t5/S32K-Knowledge-Base/RTD600-IP-S32K3X4EVB-T172-RTI-Cycle-Wake-up/ta-p/2151888)
  (S32K344 예제: PIT0 RTI, 내부 WKPU3, peripheral interrupt enable과 FIRC 전환 참고).
- [NXP S32K312 PIT RTI 논의](https://community.nxp.com/t5/S32K/Abnormal-wake-up-function-of-S32K312-PIT-RTI/td-p/2098034)
  (구버전 RTD 사례의 임의 delay를 현재 RTD에 복사하지 않는다).
