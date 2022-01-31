
#include "Board.h"
#include "GeneralProcess.h"

int main(void) {

	BoardInit();
	GpioInit();

	InitGeneralProcess();



	xTaskCreate(&TaskGeneralProcess, "GeneralProcess", 2048, NULL, 5, &GeneralProcess.TaskHandle);

	vTaskStartScheduler();
	while (true)
		;
}
