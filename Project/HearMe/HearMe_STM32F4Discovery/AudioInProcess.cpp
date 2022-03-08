
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

		uint16_t *data;
		if (xQueueReceive(PdmAudioIn.ReadyDataQueue, &data, (TickType_t)0) == pdPASS) {
			uint16_t volume = 250;
			int16_t pAudioRecBuf[INTERNAL_BUFF_SIZE / 4];

			SetPortPin(TEST1_PORT, TEST1_PIN);
			PDM_Filter_64_LSB((uint8_t *)data, (uint16_t *)pAudioRecBuf, volume, (PDMFilter_InitStruct *)&PdmAudioIn.Filter);
			ResetPortPin(TEST1_PORT, TEST1_PIN);

			for (size_t i = 0; i < sizeof(pAudioRecBuf) / sizeof(pAudioRecBuf[0]); i++) {
				int16_t val = pAudioRecBuf[i];
				PdmAudioIn.StereoBuffer[PdmAudioIn.StereoBufferSize++] = (val);
				PdmAudioIn.StereoBuffer[PdmAudioIn.StereoBufferSize++] = (val);
			}
			if (PdmAudioIn.StereoBufferSize >= sizeof(PdmAudioIn.StereoBuffer) / sizeof(PdmAudioIn.StereoBuffer[0])) {
				PlayAudioOut((uint16_t *)PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer));
				PdmAudioIn.StereoBufferSize = 0;
			}
		}
	}
}
