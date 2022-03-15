
#include "Board.h"
#include "AudioInProcess.h"
#include "I2SPdmAudioIn.h"
#include "I2SAudioOut.h"
#include "AudioSample.h"

TAudioInProcess AudioInProcess;

static void SygnalAnalysis();

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

				int16_t prevValue = PdmAudioIn.PrevValue;
				int16_t prevValueVectorized = PdmAudioIn.PrevValueVectorized;
				uint16_t maxAmplitude = 0;
				int ind = 0;
				for (size_t i = 0; i < sizeof(PdmAudioIn.DecodedBuffer) / sizeof(PdmAudioIn.DecodedBuffer[0]); i++) {
					int16_t val = PdmAudioIn.DecodedBuffer[i];

					int16_t diff = val - prevValue;
					uint16_t amplitude;
					if (diff < 0) {
						amplitude = -diff / 8;
					} else {
						amplitude = diff / 8;
					}

					if (amplitude > maxAmplitude) {
						maxAmplitude = amplitude;
						PdmAudioIn.Amplitude = ((PdmAudioIn.Amplitude * 8) + amplitude) / 9;
					}

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

					//

					if (diff > PdmAudioIn.Amplitude) {
						PdmAudioIn.Vectorized[i] = 1;
						prevValue = val;
					} else if (diff < -PdmAudioIn.Amplitude) {
						PdmAudioIn.Vectorized[i] = -1;
						prevValue = val;
					} else {
						PdmAudioIn.Vectorized[i] = 0;
					}

					PdmAudioIn.StereoBuffer[ind++] = val;
					PdmAudioIn.StereoBuffer[ind++] = val;
				}
				if (test) {
					test = false;
				}
				if (maxAmplitude > PdmAudioIn.Amplitude) {
					PdmAudioIn.Amplitude = maxAmplitude;
				} else {
					PdmAudioIn.Amplitude = ((PdmAudioIn.Amplitude * 8) + maxAmplitude) / 9;
				}
				SygnalAnalysis();

				PdmAudioIn.PrevValue = prevValue;
				PdmAudioIn.PrevValueVectorized = prevValueVectorized;
				PlayAudioOut((uint16_t *)PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer));
			}
		}
	}
}

static void SygnalAnalysis() {
	TAnalysisAudio analysisAudio;
	int counter = 0;
	int8_t prevVectorized = PdmAudioIn.PrevVectorized;
	memset(PdmAudioIn.AnalysisAudio, 0, sizeof(PdmAudioIn.AnalysisAudio));

	for (size_t i = 0; i < sizeof(PdmAudioIn.Vectorized) / sizeof(PdmAudioIn.Vectorized[0]); i++) {
		int8_t vectorized = PdmAudioIn.Vectorized[i];
		if (prevVectorized != vectorized) {

			if (counter > sizeof(PdmAudioIn.AnalysisAudio) / sizeof(PdmAudioIn.AnalysisAudio[0]) - 1) {
				counter = sizeof(PdmAudioIn.AnalysisAudio) / sizeof(PdmAudioIn.AnalysisAudio[0]) - 1;
			}

			if (prevVectorized > 0) {
				int val = PdmAudioIn.AnalysisAudio[counter].R + 1;
				if (val < UINT8_MAX) {
					PdmAudioIn.AnalysisAudio[counter].R = val;
				}
			} else if (prevVectorized < 0) {
				int val = PdmAudioIn.AnalysisAudio[counter].F + 1;
				if (val < UINT8_MAX) {
					PdmAudioIn.AnalysisAudio[counter].F = val;
				}
			} else {
				int val = PdmAudioIn.AnalysisAudio[counter].I + 1;
				if (val < UINT8_MAX) {
					PdmAudioIn.AnalysisAudio[counter].I = val;
				}
			}
			prevVectorized = vectorized;
			counter = 0;
			
		} else {
			counter++;
		}
	}
	PdmAudioIn.PrevVectorized = PdmAudioIn.Vectorized[(sizeof(PdmAudioIn.Vectorized) / sizeof(PdmAudioIn.Vectorized[0])) - 1];
}