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

#define LED_ORANGE_PIN GPIO_PIN_13
#define LED_ORANGE_PORT GPIOD

#define LED_GREEN_PIN GPIO_PIN_12
#define LED_GREEN_PORT GPIOD

#define LED_RED_PIN GPIO_PIN_14
#define LED_RED_PORT GPIOD

#define LED_BLUE_PIN GPIO_PIN_15
#define LED_BLUE_PORT GPIOD

#define BUTTON_PIN GPIO_PIN_0
#define BUTTON_PORT GPIOA

#define ReadPortPin(port, pin) ((port->IDR & pin) == pin)

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
		if (((port->ODR & pin) == pin)) {                                                                                                                      \
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