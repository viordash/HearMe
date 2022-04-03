
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
	InitFft(sizeof(PdmAudioIn.DecodedBuffer0) / sizeof(PdmAudioIn.DecodedBuffer0[0]));

	PTAudioFragmentAnalysis pAudioFragmentAnalysis;
	ReadAudioDigest0(&pAudioFragmentAnalysis);
	memcpy(&PdmAudioIn.ReferenceAudioDigest.Fragment, pAudioFragmentAnalysis, sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment));
}

void InitFft(uint32_t dataSize);
void FftAnalyze(float32_t *input, uint32_t inputSize, float32_t *output);

bool test = false;
void TaskAudioInProcess(void *arg) {
	while (true) {
		float32_t *decodedBuffer;
		if (xQueueReceive(PdmAudioIn.ReadyDecodedDataQueue, &decodedBuffer, (TickType_t)100) == pdPASS) {

			if (test) {
				test = false;
			}

			//			for (size_t i = 0; i < (sizeof(PdmAudioIn.DecodedBuffer0) / sizeof(PdmAudioIn.DecodedBuffer0[0])) / 2; i++) {
			//				int16_t val = decodedBuffer[i * 2];
			//				PdmAudioIn.StereoBuffer[i * 2] = val;
			//				PdmAudioIn.StereoBuffer[(i * 2) + 1] = val;
			//			}
			//			PlayAudioOut((uint16_t *)PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer));

			SetPortPin(TEST2_PORT, TEST2_PIN);
			FftAnalyze(decodedBuffer, PdmAudioIn.FftOutput, sizeof(PdmAudioIn.FftOutput) / sizeof(PdmAudioIn.FftOutput[0]));
			ResetPortPin(TEST2_PORT, TEST2_PIN);
		}
	}
}
