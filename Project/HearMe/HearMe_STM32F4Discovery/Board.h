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
#define LEDS_AF GPIO_AF2_TIM4

#define LED_GREEN_PIN GPIO_PIN_12
#define LED_GREEN_PORT GPIOD

#define LED_RED_PIN GPIO_PIN_14
#define LED_RED_PORT GPIOD

#define LED_BLUE_PIN GPIO_PIN_15
#define LED_BLUE_PORT GPIOD

#define BUTTON_PIN GPIO_PIN_0
#define BUTTON_PORT GPIOA

#define SPI_SCK_PIN GPIO_PIN_10
#define SPI_SCK_GPIO_PORT GPIOB
#define SPI_SCK_AF GPIO_AF5_SPI2

#define SPI_MOSI_PIN GPIO_PIN_3
#define SPI_MOSI_GPIO_PORT GPIOC
#define SPI_MOSI_AF GPIO_AF5_SPI2

#define AUDIO_RESET_PIN GPIO_PIN_4
#define AUDIO_RESET_PORT GPIOD

#define CODEC_I2C_SCL_PIN GPIO_PIN_6
#define CODEC_I2C_SCL_PORT GPIOB
#define CODEC_I2C_SCL_AF GPIO_AF4_I2C1

#define CODEC_I2C_SDA_PIN GPIO_PIN_9
#define CODEC_I2C_SDA_PORT GPIOB
#define CODEC_I2C_SDA_AF GPIO_AF4_I2C1

#define CODEC_I2S_SCK_PIN GPIO_PIN_10
#define CODEC_I2S_SCK_PORT GPIOC
#define CODEC_I2S_SCK_AF GPIO_AF6_SPI3

#define CODEC_I2S_SD_PIN GPIO_PIN_12
#define CODEC_I2S_SD_PORT GPIOC
#define CODEC_I2S_SD_AF GPIO_AF6_SPI3

#define CODEC_I2S_MCK_PIN GPIO_PIN_7
#define CODEC_I2S_MCK_PORT GPIOC
#define CODEC_I2S_MCK_AF GPIO_AF6_SPI3

#define CODEC_I2S_WS_PIN GPIO_PIN_4
#define CODEC_I2S_WS_PORT GPIOA
#define CODEC_I2S_WS_AF GPIO_AF6_SPI3

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