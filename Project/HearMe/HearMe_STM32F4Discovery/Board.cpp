#include "Board.h"

extern "C" {
extern void xPortSysTickHandler(void);
}

static void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
	PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
	PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		Error_Handler();
	}
}

extern "C" void SysTick_Handler(void) {
	HAL_IncTick();

	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
		/* Call tick handler */
		xPortSysTickHandler();
	}
}

void BoardInit(void) {
	HAL_Init();
	SystemClock_Config();
}

void GpioInit() {
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();

	GPIO_InitTypeDef gpio = {};

	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pull = GPIO_NOPULL;
	gpio.Pin = LED_BLUE_PIN;
	gpio.Alternate = LEDS_AF;
	HAL_GPIO_Init(LED_BLUE_PORT, &gpio);
	gpio.Pin = LED_RED_PIN;
	HAL_GPIO_Init(LED_RED_PORT, &gpio);
	gpio.Pin = LED_ORANGE_PIN;
	HAL_GPIO_Init(LED_ORANGE_PORT, &gpio);
	gpio.Pin = LED_GREEN_PIN;
	HAL_GPIO_Init(LED_GREEN_PORT, &gpio);

	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLDOWN;
	gpio.Pin = BUTTON_PIN;
	HAL_GPIO_Init(BUTTON_PORT, &gpio);

	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pin = SPI_SCK_PIN;
	gpio.Alternate = SPI_SCK_AF;
	HAL_GPIO_Init(SPI_SCK_GPIO_PORT, &gpio);
	gpio.Pin = SPI_MOSI_PIN;
	HAL_GPIO_Init(SPI_MOSI_GPIO_PORT, &gpio);

	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pin = AUDIO_RESET_PIN;
	HAL_GPIO_Init(AUDIO_RESET_PORT, &gpio);

	gpio.Mode = GPIO_MODE_AF_OD;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pin = CODEC_I2C_SCL_PIN;
	gpio.Alternate = CODEC_I2C_SCL_AF;
	HAL_GPIO_Init(CODEC_I2C_SCL_PORT, &gpio);
	gpio.Pin = CODEC_I2C_SDA_PIN;
	gpio.Alternate = CODEC_I2C_SDA_AF;
	HAL_GPIO_Init(CODEC_I2C_SDA_PORT, &gpio);

	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pin = CODEC_I2S_SCK_PIN;
	gpio.Alternate = CODEC_I2S_SCK_AF;
	HAL_GPIO_Init(CODEC_I2S_SCK_PORT, &gpio);
	gpio.Pin = CODEC_I2S_SD_PIN;
	gpio.Alternate = CODEC_I2S_SD_AF;
	HAL_GPIO_Init(CODEC_I2S_SD_PORT, &gpio);
	gpio.Pin = CODEC_I2S_WS_PIN;
	gpio.Alternate = CODEC_I2S_WS_AF;
	HAL_GPIO_Init(CODEC_I2S_WS_PORT, &gpio);

	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pin = CODEC_I2S_MCK_PIN;
	gpio.Alternate = CODEC_I2S_MCK_AF;
	HAL_GPIO_Init(CODEC_I2S_MCK_PORT, &gpio);
}

#ifdef __cplusplus
extern "C" {
#endif

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM4) {
		__HAL_RCC_TIM4_CLK_ENABLE();
	}
}

void HAL_MspInit(void) {
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim_pwm) {
	if (htim_pwm->Instance == TIM4) {
		__HAL_RCC_TIM4_CLK_ENABLE();
	}
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
	//	if (huart->Instance == USART3) {
	//		InitUart3Msp(huart);
	//	}
	//	if (huart->Instance == USART2) {
	//		InitUart2Msp(huart);
	//	}
}

void HAL_I2S_MspInit(I2S_HandleTypeDef *hi2s) {
}

#ifdef __cplusplus
}
#endif