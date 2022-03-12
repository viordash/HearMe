
#include "Board.h"
#include "GeneralProcess.h"
#include "AudioInProcess.h"

int main(void) {

	BoardInit();
	GpioInit();

	InitGeneralProcess();
	InitAudioInProcess();

	xTaskCreate(&TaskGeneralProcess, "GeneralProcess", 2048, NULL, tskIDLE_PRIORITY, &GeneralProcess.TaskHandle);
	xTaskCreate(&TaskAudioInProcess, "AudioInProcess", 4096, NULL, tskIDLE_PRIORITY+1, &AudioInProcess.TaskHandle);

	vTaskStartScheduler();
	while (true)
		;
}
