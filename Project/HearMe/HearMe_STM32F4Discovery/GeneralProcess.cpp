
#include "Board.h"
#include "GeneralProcess.h"
#include "Button.h"
#include "Leds.h"
#include "I2SPdmAudioIn.h"
#include "I2SAudioOut.h"

TGeneralProcess GeneralProcess;

extern uint16_t AUDIO_SAMPLE[];
/* Audio file size and start address are defined here since the audio file is
	stored in Flash memory as a constant table of 16-bit data */
#define AUDIO_FILE_SIZE 990000
#define AUIDO_START_ADDRESS 58 /* Offset relative to audio file header size */

void InitGeneralProcess() {
	memset(&GeneralProcess, 0, sizeof(GeneralProcess));
	InitButton();
	InitLeds();
	InitPdmAudioIn();
	InitAudioOut();
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
//					StartPdmAudioIn();
					StartAudioOut((uint16_t *)(AUDIO_SAMPLE + AUIDO_START_ADDRESS), AUDIO_FILE_SIZE - AUIDO_START_ADDRESS);
				} else {
					ChangeOrangeLed(TLedMode::Off);
//					StopPdmAudioIn();
					StopAudioOut(TCodecPowerdown::CODEC_PDWN_SW);
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
