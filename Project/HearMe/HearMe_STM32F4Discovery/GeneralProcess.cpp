
#include "Board.h"
#include "GeneralProcess.h"

TGeneralProcess GeneralProcess;

void InitGeneralProcess() {
	memset(&GeneralProcess, 0, sizeof(GeneralProcess));
}

void TaskGeneralProcess(void *arg) {
	GeneralProcess.timers.timerHeartbeat = SysTickCount - SYSTICK_mS(2000);

	while (true) {

		SetPortPin(GPIOD, GPIO_PIN_15);

		TaskSleep(SYSTICK_mS(200));

		ResetPortPin(GPIOD, GPIO_PIN_15);

		TaskSleep(SYSTICK_mS(200));

		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);

		TaskSleep(SYSTICK_mS(50));
	}
}
