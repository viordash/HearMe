
#include "Board.h"
#include "AudioInProcess.h"
#include "I2SPdmAudioIn.h"
#include "I2SAudioOut.h"
#include "AudioSample.h"

TAudioInProcess AudioInProcess;

static void SygnalAnalysis();
static uint8_t AverageAudio(uint8_t stored, uint8_t current);
static void AudioMatching();
static uint16_t Match(uint8_t reference, uint8_t analysed);

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

					//

					if (diff > PdmAudioIn.PrevDiff) {
						PdmAudioIn.Vectorized[i] = 1;
						prevValue = val;
					} else if (diff < -PdmAudioIn.PrevDiff) {
						PdmAudioIn.Vectorized[i] = -1;
						prevValue = val;
					} else {
						PdmAudioIn.Vectorized[i] = 0;
					}
					if (diff < 0) {
						PdmAudioIn.PrevDiff = -diff / 4;
					} else {
						PdmAudioIn.PrevDiff = diff / 4;
					}

					PdmAudioIn.StereoBuffer[ind++] = val;
					PdmAudioIn.StereoBuffer[ind++] = val;
				}
				if (test) {
					test = false;
				}

				SetPortPin(TEST2_PORT, TEST2_PIN);
				SygnalAnalysis();
				AudioMatching();
				ResetPortPin(TEST2_PORT, TEST2_PIN);

				PdmAudioIn.PrevValue = prevValue;
				PdmAudioIn.PrevValueVectorized = prevValueVectorized;
				PlayAudioOut((uint16_t *)PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer));
			}
		}
	}
}

static void SygnalAnalysis() {
	int counter = 0;
	int8_t prevVectorized = PdmAudioIn.PrevVectorized;
	TAnalysisAudio analysisAudio[sizeof(PdmAudioIn.AnalysisAudio) / sizeof(PdmAudioIn.AnalysisAudio[0])] = {};

	for (size_t i = 0; i < sizeof(PdmAudioIn.Vectorized) / sizeof(PdmAudioIn.Vectorized[0]); i++) {
		int8_t vectorized = PdmAudioIn.Vectorized[i];
		if (prevVectorized != vectorized) {

			if (counter > sizeof(analysisAudio) / sizeof(analysisAudio[0]) - 1) {
				counter = sizeof(analysisAudio) / sizeof(analysisAudio[0]) - 1;
			}

			if (prevVectorized > 0) {
				int val = analysisAudio[counter].R + 1;
				if (val < UINT8_MAX) {
					analysisAudio[counter].R = val;
				}
			} else if (prevVectorized < 0) {
				int val = analysisAudio[counter].F + 1;
				if (val < UINT8_MAX) {
					analysisAudio[counter].F = val;
				}
			} else {
				int val = analysisAudio[counter].I + 1;
				if (val < UINT8_MAX) {
					analysisAudio[counter].I = val;
				}
			}
			prevVectorized = vectorized;
			counter = 0;
		} else {
			counter++;
		}
	}

	for (size_t i = 0; i < sizeof(analysisAudio) / sizeof(analysisAudio[0]); i++) {
		PdmAudioIn.AnalysisAudio[i].R = AverageAudio(PdmAudioIn.AnalysisAudio[i].R, analysisAudio[i].R);
		PdmAudioIn.AnalysisAudio[i].I = AverageAudio(PdmAudioIn.AnalysisAudio[i].I, analysisAudio[i].I);
		PdmAudioIn.AnalysisAudio[i].F = AverageAudio(PdmAudioIn.AnalysisAudio[i].F, analysisAudio[i].F);
	}

	PdmAudioIn.PrevVectorized = PdmAudioIn.Vectorized[(sizeof(PdmAudioIn.Vectorized) / sizeof(PdmAudioIn.Vectorized[0])) - 1];
}

static uint8_t AverageAudio(uint8_t stored, uint8_t current) {
	if (current == 0) {
		return 0;
	}
	if (current > 8) {
		return (((uint32_t)stored * (uint32_t)8) + (uint32_t)current) / (uint32_t)9;
	}
	return (((uint32_t)stored * (uint32_t)(current - 1)) + (uint32_t)current) / (uint32_t)current;
}

static void AudioMatching() {
	uint16_t equability = 0;
	for (size_t i = 0; i < sizeof(PdmAudioIn.AnalysisAudio) / sizeof(PdmAudioIn.AnalysisAudio[0]); i++) {
		equability += Match(PdmAudioIn.ReferenceAudio[i].R, PdmAudioIn.AnalysisAudio[i].R);
		equability += Match(PdmAudioIn.ReferenceAudio[i].I, PdmAudioIn.AnalysisAudio[i].I);
		equability += Match(PdmAudioIn.ReferenceAudio[i].F, PdmAudioIn.AnalysisAudio[i].F);
	}
	PdmAudioIn.Equability = (PdmAudioIn.Equability + (equability * 4)) / 5;
}

static uint16_t Match(uint8_t reference, uint8_t analysed) {
	if (reference == 0) {
		return 0;
	}

	int diff = reference - analysed;

	if (diff > 0) {
		if (diff < reference / 4) {
			return reference;
		}
	} else if (diff < 0) {
		if (-diff < analysed / 4) {
			return analysed;
		}
	} else {
		return reference;
	}
	return 0;
}