
#include "Board.h"
#include "GeneralProcess.h"
#include "Button.h"
#include "Leds.h"
#include "I2SPdmAudioIn.h"
#include "I2SAudioOut.h"
#include "AudioSample.h"

TGeneralProcess GeneralProcess;

#define AUIDO_START_ADDRESS 58 /* Offset relative to audio file header size */

void InitGeneralProcess() {
	memset(&GeneralProcess, 0, sizeof(GeneralProcess));
	InitButton();
	InitLeds();
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
					//					StartAudioOut((uint16_t *)(AUDIO_SAMPLE + AUIDO_START_ADDRESS), sizeof(AUDIO_SAMPLE) - AUIDO_START_ADDRESS, 80, true);
				} else {
					ChangeOrangeLed(TLedMode::Off);
					StopPdmAudioIn();
					StopAudioOut();
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
