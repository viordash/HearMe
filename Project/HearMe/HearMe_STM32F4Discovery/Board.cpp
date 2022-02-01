#include "Board.h"

extern "C" {
extern void xPortSysTickHandler(void);
}

static void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 8;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
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
	__GPIOD_CLK_ENABLE();
	
	GPIO_InitTypeDef gpio;

	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pull = GPIO_NOPULL;

	gpio.Pin = LED_BLUE_PIN;
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
}