
#include "Board.h"
#include "AudioInProcess.h"
#include "I2SAudioOut.h"
#include "AudioSample.h"
#include "FlashMemStorage.h"

TAudioInProcess AudioInProcess;

static void SygnalAnalysis();
static uint8_t AverageAudio(uint8_t stored, uint8_t current);
static void AudioMatching();
static bool Match(uint8_t reference, uint8_t analysed);

void InitAudioInProcess() {
	memset(&AudioInProcess, 0, sizeof(AudioInProcess));
	InitPdmAudioIn();
	InitAudioOut();

	PTAudioFragmentAnalysis pAudioFragmentAnalysis;
	ReadAudioDigest0(&pAudioFragmentAnalysis);
	memcpy(&PdmAudioIn.ReferenceAudioDigest.Fragment, pAudioFragmentAnalysis, sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment));
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

				uint32_t amplitude = 0;
				int16_t prevValue = PdmAudioIn.PrevValue;
				int16_t prevValueVectorized = PdmAudioIn.PrevValueVectorized;
				int ind = 0;
				for (size_t i = 0; i < sizeof(PdmAudioIn.DecodedBuffer) / sizeof(PdmAudioIn.DecodedBuffer[0]); i++) {
					int16_t val = PdmAudioIn.DecodedBuffer[i];

					int16_t diff = val - prevValue;

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
						if (amplitude < -diff) {
							amplitude = -diff;
						}
					} else {
						PdmAudioIn.PrevDiff = diff / 4;
						if (amplitude < diff) {
							amplitude = diff;
						}
					}

//					PdmAudioIn.StereoBuffer[ind++] = val;
//					PdmAudioIn.StereoBuffer[ind++] = val;
				}

				PdmAudioIn.Amplitude = (PdmAudioIn.Amplitude + (amplitude * 8)) / 9;
				if (PdmAudioIn.Amplitude < 500) {
					memset(PdmAudioIn.Vectorized, 0, sizeof(PdmAudioIn.Vectorized));
				} else if (PdmAudioIn.RequestToStoreReferenceAudio) {
					PdmAudioIn.RequestToStoreReferenceAudio = false;
					PdmAudioIn.StoreReferenceAudio = true;
					PdmAudioIn.ReferenceAudioDigest.MatchedFramesCount = 0;
					PdmAudioIn.ReferenceAudioDigest.Index = 0;
				}

				SetPortPin(TEST2_PORT, TEST2_PIN);
				SygnalAnalysis();
				if (PdmAudioIn.StoreReferenceAudio) {
					memcpy(&PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[PdmAudioIn.ReferenceAudioDigest.Index], &PdmAudioIn.CurrentAudioDigest,
						   sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[PdmAudioIn.ReferenceAudioDigest.Index]));
					PdmAudioIn.ReferenceAudioDigest.Index++;
					if (PdmAudioIn.ReferenceAudioDigest.Index
						>= sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames) / sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[0])) {
						PdmAudioIn.StoreReferenceAudio = false;
						PdmAudioIn.ReferenceAudioDigest.Index = 0;
						WriteAudioDigest0(&PdmAudioIn.ReferenceAudioDigest.Fragment);
					}
				} else {
					AudioMatching();
				}
				ResetPortPin(TEST2_PORT, TEST2_PIN);

				PdmAudioIn.PrevValue = prevValue;
				PdmAudioIn.PrevValueVectorized = prevValueVectorized;
//				PlayAudioOut((uint16_t *)PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer));
			}
		}
	}
}

static void SygnalAnalysis() {
	int counter = 0;
	int8_t prevVectorized = PdmAudioIn.PrevVectorized;
	TAudioDigest audioDigest = {};

	for (size_t i = 0; i < sizeof(PdmAudioIn.Vectorized) / sizeof(PdmAudioIn.Vectorized[0]); i++) {
		int8_t vectorized = PdmAudioIn.Vectorized[i];
		if (prevVectorized != vectorized) {

			if (counter > sizeof(audioDigest.Samples) / sizeof(audioDigest.Samples[0]) - 1) {
				counter = sizeof(audioDigest.Samples) / sizeof(audioDigest.Samples[0]) - 1;
			}

			if (prevVectorized > 0) {
				int val = audioDigest.Samples[counter].R + 1;
				if (val < UINT8_MAX) {
					audioDigest.Samples[counter].R = val;
				}
			} else if (prevVectorized < 0) {
				int val = audioDigest.Samples[counter].F + 1;
				if (val < UINT8_MAX) {
					audioDigest.Samples[counter].F = val;
				}
			} else {
				int val = audioDigest.Samples[counter].I + 1;
				if (val < UINT8_MAX) {
					audioDigest.Samples[counter].I = val;
				}
			}
			prevVectorized = vectorized;
			counter = 0;
		} else {
			counter++;
		}
	}

	for (size_t i = 0; i < sizeof(audioDigest.Samples) / sizeof(audioDigest.Samples[0]); i++) {
		PdmAudioIn.CurrentAudioDigest.Samples[i].R = AverageAudio(PdmAudioIn.CurrentAudioDigest.Samples[i].R, audioDigest.Samples[i].R);
		PdmAudioIn.CurrentAudioDigest.Samples[i].I = AverageAudio(PdmAudioIn.CurrentAudioDigest.Samples[i].I, audioDigest.Samples[i].I);
		PdmAudioIn.CurrentAudioDigest.Samples[i].F = AverageAudio(PdmAudioIn.CurrentAudioDigest.Samples[i].F, audioDigest.Samples[i].F);
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
	if (test) {
		test = false;
	}
	for (size_t i = 0; i < sizeof(PdmAudioIn.CurrentAudioDigest.Samples) / sizeof(PdmAudioIn.CurrentAudioDigest.Samples[0]); i++) {
		PTAudioDigest referenceFrame = &PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[PdmAudioIn.ReferenceAudioDigest.Index];
		PTAnalysisAudioSample pReference = &referenceFrame->Samples[i];
		PTAnalysisAudioSample pSampleCurrent = &PdmAudioIn.CurrentAudioDigest.Samples[i];
		PTAnalysisAudioSample pSamplePrior = i > 0 //
												 ? &PdmAudioIn.CurrentAudioDigest.Samples[i - 1]
												 : NULL;
		PTAnalysisAudioSample pSampleNext
			= i < (sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames) / sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[0])) - 1 //
				  ? &PdmAudioIn.CurrentAudioDigest.Samples[i + 1]
				  : NULL;

		if (pReference->R > 0) {
			if (Match(pReference->R, pSampleCurrent->R)							   //
				|| (pSamplePrior != NULL && Match(pReference->R, pSamplePrior->R)) //
				|| (pSampleNext != NULL && Match(pReference->R, pSampleNext->R))) {
				equability++;
			}
		}

		if (pReference->I > 0) {
			if (Match(pReference->I, pSampleCurrent->I)							   //
				|| (pSamplePrior != NULL && Match(pReference->I, pSamplePrior->I)) //
				|| (pSampleNext != NULL && Match(pReference->I, pSampleNext->I))) {
				equability++;
			}
		}

		if (pReference->F > 0) {
			if (Match(pReference->F, pSampleCurrent->F)							   //
				|| (pSamplePrior != NULL && Match(pReference->F, pSamplePrior->F)) //
				|| (pSampleNext != NULL && Match(pReference->F, pSampleNext->F))) {
				equability++;
			}
		}
	}
	PdmAudioIn.Equability = equability;

	if (PdmAudioIn.Equability >= 3) {
		PdmAudioIn.ReferenceAudioDigest.Index++;
		if (PdmAudioIn.ReferenceAudioDigest.Index
			>= sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames) / sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[0])) {
			PdmAudioIn.ReferenceAudioDigest.Index = 0;
		}
	} else {
		PdmAudioIn.ReferenceAudioDigest.Index = 0;
	}
}

static bool Match(uint8_t reference, uint8_t analysed) {
	int diff = reference - analysed;

	if (diff > 0) {
		if (diff < reference / 4) {
			return true;
		}
	} else if (diff < 0) {
		if (-diff < analysed / 4) {
			return true;
		}
	} else {
		return true;
	}
	return 0;
}