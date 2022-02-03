
#include "Board.h"
#include "GeneralProcess.h"
#include "Button.h"
#include "Leds.h"
#include "I2SPdmAudioIn.h"

TGeneralProcess GeneralProcess;

void InitGeneralProcess() {
	memset(&GeneralProcess, 0, sizeof(GeneralProcess));
	InitButton();
	InitLeds();
	InitPdmAudioIn();
}

void TaskGeneralProcess(void *arg) {
	StartLeds();
	GeneralProcess.timers.timerHeartbeat = SysTickCount - SYSTICK_mS(2000);

	bool d = false;
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
				d = !d;
				if (d) {
					ChangeOrangeLed(TLedMode::FastFlash);
					StartPdmAudioIn();
				} else {
					ChangeOrangeLed(TLedMode::Off);
					StopPdmAudioIn();
				}

				break;

			case TButtonState::MidPress:
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
