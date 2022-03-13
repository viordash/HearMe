
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

bool test = false;
void TaskAudioInProcess(void *arg) {
	while (true) {
		int16_t *data;
		if (xQueueReceive(PdmAudioIn.ReadyDataQueue, &data, (TickType_t)100) == pdPASS) {
			uint16_t pAudioRecBuf[16];
			SetPortPin(TEST1_PORT, TEST1_PIN);
			PDM_Filter_64_LSB((uint8_t *)data, (uint16_t *)pAudioRecBuf, PdmAudioIn.MicLevel, (PDMFilter_InitStruct *)&PdmAudioIn.Filter);
			ResetPortPin(TEST1_PORT, TEST1_PIN);

			for (size_t i = 0; i < sizeof(pAudioRecBuf) / sizeof(pAudioRecBuf[0]); i++) {
				int16_t val = pAudioRecBuf[i];
				PdmAudioIn.DecodedBuffer[PdmAudioIn.DecodedBufferSize++] = val;
			}

			if (PdmAudioIn.DecodedBufferSize >= sizeof(PdmAudioIn.DecodedBuffer) / sizeof(PdmAudioIn.DecodedBuffer[0])) {
				PdmAudioIn.DecodedBufferSize = 0;

				volatile int16_t prevValue = PdmAudioIn.PrevValue;
				volatile int16_t prevValueVectorized = PdmAudioIn.PrevValueVectorized;
				int ind = 0;
				for (size_t i = 0; i < sizeof(PdmAudioIn.DecodedBuffer) / sizeof(PdmAudioIn.DecodedBuffer[0]); i++) {
					int16_t val = PdmAudioIn.DecodedBuffer[i];

					int16_t diff = val - prevValue;

					//					int16_t vectorized;
					//					if (diff > 1000) {
					//						vectorized = prevValueVectorized + 1;
					//						prevValue = val;
					//					} else if (diff < -1000) {
					//						vectorized = prevValueVectorized - 1;
					//						prevValue = val;
					//					} else {
					//						vectorized = prevValueVectorized;
					//					}
					//					if (vectorized > INT8_MAX) {
					//						vectorized = INT8_MAX;
					//					} else if (vectorized < INT8_MIN) {
					//						vectorized = INT8_MIN;
					//					}
					//					PdmAudioIn.Vectorized[i] = vectorized;
					//					prevValueVectorized = vectorized;

					if (diff > 1000) {
						PdmAudioIn.Vectorized[i] = 1;
						prevValue = val;
					} else if (diff < -1000) {
						PdmAudioIn.Vectorized[i] = -1;
						prevValue = val;
					} else {
						PdmAudioIn.Vectorized[i] = 0;
					}

					PdmAudioIn.StereoBuffer[ind++] = val;
					PdmAudioIn.StereoBuffer[ind++] = val;
				}
				PdmAudioIn.PrevValue = prevValue;
				PdmAudioIn.PrevValueVectorized = prevValueVectorized;
				PlayAudioOut((uint16_t *)PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer));
				if (test) {
					test = false;
				}
			}
		}
	}
}
