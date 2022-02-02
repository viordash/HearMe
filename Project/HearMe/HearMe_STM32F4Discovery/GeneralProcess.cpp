
#include "Board.h"
#include "GeneralProcess.h"
#include "Button.h"
#include "Leds.h"

TGeneralProcess GeneralProcess;

void InitGeneralProcess() {
	memset(&GeneralProcess, 0, sizeof(GeneralProcess));
	InitButton();
	InitLeds();
}

void TaskGeneralProcess(void *arg) {
	StartLeds();
	GeneralProcess.timers.timerHeartbeat = SysTickCount - SYSTICK_mS(2000);

	while (true) {
		switch (ButtonPeriodic()) {
			case TButtonState::Pressed:
				ChangeBlueLed(TLedMode::Br50);
				break;
			case TButtonState::Released:
				ChangeBlueLed(TLedMode::Br1);
				break;

			case TButtonState::OnKeyDown:
				ChangeRedLed(TLedMode::Pulse);
				break;

			case TButtonState::ShortPress:
				ChangeOrangeLed(TLedMode::Flash);
				break;

			case TButtonState::MidPress:
				ChangeOrangeLed(TLedMode::FastFlash);
				break;

			case TButtonState::LongPress:
				ChangeGreenLed(TLedMode::FastFastFlash);
				break;

			default:
				break;
		}

		TaskSleep(SYSTICK_mS(1));
	}
}
