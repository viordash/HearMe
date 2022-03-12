
#include "Board.h"
#include "AudioInProcess.h"
#include "I2SPdmAudioIn.h"
#include "I2SAudioOut.h"
#include "AudioSample.h"

TAudioInProcess AudioInProcess;

#define AUIDO_START_ADDRESS 58 /* Offset relative to audio file header size */

void InitAudioInProcess() {
	memset(&AudioInProcess, 0, sizeof(AudioInProcess));
	InitPdmAudioIn();
	InitAudioOut();
}

void TaskAudioInProcess(void *arg) {
	while (true) {
		int16_t *data;
		if (xQueueReceive(PdmAudioIn.ReadyDataQueue, &data, (TickType_t)10) == pdPASS) {
			uint16_t pAudioRecBuf[16];
			SetPortPin(TEST1_PORT, TEST1_PIN);
			PDM_Filter_64_LSB((uint8_t *)data, (uint16_t *)pAudioRecBuf, PdmAudioIn.MicLevel, (PDMFilter_InitStruct *)&PdmAudioIn.Filter);
			ResetPortPin(TEST1_PORT, TEST1_PIN);

			for (size_t i = 0; i < sizeof(pAudioRecBuf) / sizeof(pAudioRecBuf[0]); i++) {
				int16_t val = pAudioRecBuf[i];
				PdmAudioIn.StereoBuffer[PdmAudioIn.StereoBufferSize++] = (val);
				PdmAudioIn.StereoBuffer[PdmAudioIn.StereoBufferSize++] = (val);
			}
			if (PdmAudioIn.StereoBufferSize >= sizeof(PdmAudioIn.StereoBuffer) / sizeof(PdmAudioIn.StereoBuffer[0])) {
				PlayAudioOut((uint16_t *)PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer));
				TogglePortPin(TEST2_PORT, TEST2_PIN);
				PdmAudioIn.StereoBufferSize = 0;
			}
		}
	}
}
