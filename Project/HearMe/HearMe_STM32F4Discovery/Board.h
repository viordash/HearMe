#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <stm32f4xx_hal.h>
#include "FreeRTOS.h"
#include "task.h"

#include "SystemOS.h"

#define ReadPortPin(port, pin) ((port->ODR & pin) == pin)
#define WritePortPin(port, pin, value)                                                                                                                         \
	{                                                                                                                                                          \
		if (value) {                                                                                                                                           \
			SetPortPin(port, pin);                                                                                                                             \
		} else {                                                                                                                                               \
			ResetPortPin(port, pin);                                                                                                                           \
		}                                                                                                                                                      \
	}

#define SetPortPin(port, pin) WRITE_REG(port->BSRR, pin)
#define ResetPortPin(port, pin) WRITE_REG(port->BSRR, (pin << 16))

#define TogglePortPin(port, pin)                                                                                                                               \
	{                                                                                                                                                          \
		if (ReadPortPin(port, pin)) {                                                                                                                          \
			ResetPortPin(port, pin);                                                                                                                           \
		} else {                                                                                                                                               \
			SetPortPin(port, pin);                                                                                                                             \
		}                                                                                                                                                      \
	}

inline void Error_Handler() {
	while (!0)
		;
}

void BoardInit(void);

void GpioInit();