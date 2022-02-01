
#include "Board.h"
#include "GeneralProcess.h"
#include "Button.h"

TGeneralProcess GeneralProcess;

void InitGeneralProcess() {
	memset(&GeneralProcess, 0, sizeof(GeneralProcess));
	InitButton();
}

void TaskGeneralProcess(void *arg) {
	GeneralProcess.timers.timerHeartbeat = SysTickCount - SYSTICK_mS(2000);

	while (true) {
		switch (ButtonPeriodic()) {
			case TButtonState::Pressed:
				SetPortPin(LED_BLUE_PORT, LED_BLUE_PIN);
				break;
			case TButtonState::Released:
				ResetPortPin(LED_BLUE_PORT, LED_BLUE_PIN);
				break;

			case TButtonState::OnKeyDown:
				TogglePortPin(LED_ORANGE_PORT, LED_ORANGE_PIN);
				break;

			case TButtonState::ShortPress:
				TogglePortPin(LED_RED_PORT, LED_RED_PIN);
				break;

			case TButtonState::MidPress:
				TogglePortPin(LED_GREEN_PORT, LED_GREEN_PIN);
				break;

			case TButtonState::LongPress:
				//				TogglePortPin(LED_BLUE_PORT, LED_BLUE_PIN);
				break;

			default:
				break;
		}

		TaskSleep(SYSTICK_mS(1));
	}
}
