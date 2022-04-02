
#include "Board.h"
#include "AudioInProcess.h"
#include "I2SAudioOut.h"
#include "AudioSample.h"
#include "FlashMemStorage.h"
#include "FftAnalyze.h"

TAudioInProcess AudioInProcess;

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

void InitFft(uint32_t dataSize);
void FftAnalyze(float32_t *input, uint32_t inputSize, float32_t *output);

bool test = false;
void TaskAudioInProcess(void *arg) {

	InitFft(sizeof(PdmAudioIn.DecodedBuffer) / sizeof(PdmAudioIn.DecodedBuffer[0]));
	while (true) {

		int16_t *data;
		if (xQueueReceive(PdmAudioIn.ReadyDataQueue, &data, (TickType_t)100) == pdPASS) {
			int16_t pAudioRecBuf[16];
			SetPortPin(TEST1_PORT, TEST1_PIN);
			PDM_Filter_64_LSB((uint8_t *)data, (uint16_t *)pAudioRecBuf, PdmAudioIn.MicLevel, (PDMFilter_InitStruct *)&PdmAudioIn.Filter);
			ResetPortPin(TEST1_PORT, TEST1_PIN);

			for (size_t i = 0; i < sizeof(pAudioRecBuf) / sizeof(pAudioRecBuf[0]); i++) {
				float32_t val = pAudioRecBuf[i];
				PdmAudioIn.DecodedBuffer[PdmAudioIn.DecodedBufferSize++] = val;
				PdmAudioIn.DecodedBuffer[PdmAudioIn.DecodedBufferSize++] = 0;
			}

			if (PdmAudioIn.DecodedBufferSize >= sizeof(PdmAudioIn.DecodedBuffer) / sizeof(PdmAudioIn.DecodedBuffer[0])) {
				PdmAudioIn.DecodedBufferSize = 0;

				//				int ind = 0;
				//				for (size_t i = 0; i < sizeof(PdmAudioIn.DecodedBuffer) / sizeof(PdmAudioIn.DecodedBuffer[0]); i++) {
				//					int16_t val = PdmAudioIn.DecodedBuffer[i];
				//					PdmAudioIn.StereoBuffer[ind++] = val;
				//					PdmAudioIn.StereoBuffer[ind++] = val;
				//				}
				//				PlayAudioOut((uint16_t *)PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer));

				if (test) {
					test = false;
				}

				SetPortPin(TEST2_PORT, TEST2_PIN);
				FftAnalyze(PdmAudioIn.DecodedBuffer, PdmAudioIn.FftOutput, sizeof(PdmAudioIn.FftOutput) / sizeof(PdmAudioIn.FftOutput[0]));
				ResetPortPin(TEST2_PORT, TEST2_PIN);
			}
		}
	}
}
