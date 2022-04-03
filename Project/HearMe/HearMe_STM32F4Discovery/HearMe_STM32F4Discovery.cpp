
#include "Board.h"
#include "GeneralProcess.h"
#include "AudioInProcess.h"
#include "I2SPdmAudioIn.h"

int main(void) {

	BoardInit();
	GpioInit();

	InitGeneralProcess();
	InitAudioInProcess();

	xTaskCreate(&TaskGeneralProcess, "GeneralProcess", 2048, NULL, tskIDLE_PRIORITY, &GeneralProcess.TaskHandle);
	xTaskCreate(&TaskPdmAudioDecode, "PdmAudioDecode", 2048, NULL, tskIDLE_PRIORITY, &PdmAudioIn.TaskHandle);
	xTaskCreate(&TaskAudioInProcess, "AudioInProcess", 2048, NULL, tskIDLE_PRIORITY, &AudioInProcess.TaskHandle);

	vTaskStartScheduler();
	while (true)
		;
}
