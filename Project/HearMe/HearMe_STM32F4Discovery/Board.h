#ifndef __BOARD_H
#define __BOARD_H

#include <stm32f4xx_hal.h>
#include "FreeRTOS.h"
#include "task.h"

#define GetTickCount HAL_GetTick
#define TaskSleep(ADelayMS) osDelay(ADelayMS)
#define TaskSleepZero() osThreadYield()

#define ReadPortPin(APort, APin) (*((BYTE *)(PERIPH_BB_BASE + (((DWORD)(((DWORD) & (APort->IDR)) - PERIPH_BASE) * 0x20) + (APin * 4)))))
#define WritePortPin(APort, APin, AValue) (*((BYTE *)(PERIPH_BB_BASE + (((DWORD)(((DWORD) & (APort->ODR)) - PERIPH_BASE) * 0x20) + (APin * 4)))) = AValue)

#define SetPortPin(APort, APin) (*((BYTE *)(PERIPH_BB_BASE + (((DWORD)(((DWORD) & (APort->BSRR)) - PERIPH_BASE) * 0x20) + (APin * 4)))) = 1)
#define ResetPortPin(APort, APin) (*((BYTE *)(PERIPH_BB_BASE + (((DWORD)(((DWORD) & (APort->BRR)) - PERIPH_BASE) * 0x20) + (APin * 4)))) = 1)

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

inline void Error_Handler() {
	while (!0)
		;
}

void BoardInit(void);

#endif /* end __BOARD_H */