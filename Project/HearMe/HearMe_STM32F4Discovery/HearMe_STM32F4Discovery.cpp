
#include "Board.h"

static void LED_Thread1(void *argument);
static void LED_Thread2(void *argument);

TaskHandle_t LEDThread1Handle;
TaskHandle_t LEDThread2Handle;

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {

	BoardInit();

	__GPIOD_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Pin = GPIO_PIN_15 | GPIO_PIN_14;

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* Thread 1 definition */

	xTaskCreate(LED_Thread1, "LED1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &LEDThread1Handle);
	xTaskCreate(LED_Thread2, "LED2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &LEDThread2Handle);

	vTaskStartScheduler();

	/* We should never get here as control is now taken by the scheduler */
	while (true)
		;
}

/**
 * @brief  Toggle LED1
 * @param  thread not used
 * @retval None
 */
static void LED_Thread1(void *argument) {

	for (;;) {
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
		vTaskDelay(2000 / portTICK_PERIOD_MS);

		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
		vTaskSuspend(LEDThread2Handle);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		vTaskResume(LEDThread2Handle);
	}
}

/**
 * @brief  Toggle LED2 thread
 * @param  argument not used
 * @retval None
 */
static void LED_Thread2(void *argument) {
	uint32_t count;

	for (;;) {
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1) {
	}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
