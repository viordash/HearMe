#pragma once

#define SYSTICK_PERIOD_uS (1000)
#define SYSTICK_mS(x) ((x) * (1000 / SYSTICK_PERIOD_uS))
#define SYSTICK_2_MS(x) ((x * SYSTICK_PERIOD_uS) / 1000)
#define GetCurrentTimeSeconds() (SysTickCount / ((1000 / SYSTICK_PERIOD_uS) * 1000))

typedef void (*TaskSleepCb)(uint32_t);
#define SysTickCount xTaskGetTickCount()

#define TaskSleep(timeMs)                                                                                                                                      \
	{ vTaskDelay(pdMS_TO_TICKS(timeMs)); }

#define TaskSleepZero() taskYIELD()

#define Disable_Interrupts() taskDISABLE_INTERRUPTS()
#define Enable_Interrupts() taskENABLE_INTERRUPTS()

#define Delay_nS(ACount_nS)                                                                                                                                    \
	{                                                                                                                                                          \
		DWORD i = (SystemCoreClock / 100000000) * (ACount_nS / 10);                                                                                            \
		while (i--)                                                                                                                                            \
			__NOP();                                                                                                                                           \
	}
#define Delay_uS(ACount_uS)                                                                                                                                    \
	{                                                                                                                                                          \
		DWORD i = (SystemCoreClock / 1000000) * ACount_uS;                                                                                                     \
		while (i--)                                                                                                                                            \
			__NOP();                                                                                                                                           \
	}