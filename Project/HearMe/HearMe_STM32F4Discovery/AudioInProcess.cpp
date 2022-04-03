
#include "Board.h"
#include "AudioInProcess.h"
#include "I2SAudioOut.h"
#include "AudioSample.h"
#include "FlashMemStorage.h"
#include "FftAnalyze.h"
#include "Leds.h"

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

			//			for (size_t i = 0; i < (sizeof(PdmAudioIn.DecodedBuffer0) / sizeof(PdmAudioIn.DecodedBuffer0[0])) / 2; i++) {
			//				int16_t val = decodedBuffer[i * 2];
			//				PdmAudioIn.StereoBuffer[i * 2] = val;
			//				PdmAudioIn.StereoBuffer[(i * 2) + 1] = val;
			//			}
			//			PlayAudioOut((uint16_t *)PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer));

			SetPortPin(TEST2_PORT, TEST2_PIN);
			const uint32_t magnitudeSize = sizeof(PdmAudioIn.FftMagnitude) / sizeof(PdmAudioIn.FftMagnitude[0]);
			FftAnalyze(decodedBuffer, PdmAudioIn.FftMagnitude, magnitudeSize);
			ResetPortPin(TEST2_PORT, TEST2_PIN);

			float32_t maxBinValue;
			float32_t binValue;
			uint32_t binIndex;

			memset(PdmAudioIn.CurrentAudioDigest.FftBins, 0, sizeof(PdmAudioIn.CurrentAudioDigest.FftBins));
			GetMaxMagnitude(PdmAudioIn.FftMagnitude, magnitudeSize, &maxBinValue, &binIndex);
			binValue = maxBinValue;
			const float32_t cutOff = maxBinValue / 20;

			while (binValue > cutOff) {
				uint32_t startIndex;
				uint32_t endIndex;

				if (!GetBinBorders(PdmAudioIn.FftMagnitude, magnitudeSize, binIndex, &startIndex, &endIndex)
					|| binIndex > sizeof(PdmAudioIn.CurrentAudioDigest.FftBins) / sizeof(PdmAudioIn.CurrentAudioDigest.FftBins[0])) {
					break;
				}
				for (size_t k = startIndex; k <= endIndex; k++) {
					PdmAudioIn.FftMagnitude[k] = 0;
				}
				PdmAudioIn.CurrentAudioDigest.FftBins[binIndex] = (binValue * 255) / maxBinValue;

				GetMaxMagnitude(PdmAudioIn.FftMagnitude, magnitudeSize, &binValue, &binIndex);
			}

			PdmAudioIn.MaxBinValue = (PdmAudioIn.MaxBinValue + (maxBinValue * 8)) / 9;
			if (PdmAudioIn.RequestToStoreReferenceAudio && PdmAudioIn.MaxBinValue > 300000) {
				PdmAudioIn.RequestToStoreReferenceAudio = false;
				PdmAudioIn.StoreReferenceAudio = true;
				PdmAudioIn.ReferenceAudioDigest.MatchedFramesCount = 0;
				PdmAudioIn.ReferenceAudioDigest.Index = 0;
			}

			if (PdmAudioIn.StoreReferenceAudio) {
				memcpy(&PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[PdmAudioIn.ReferenceAudioDigest.Index], &PdmAudioIn.CurrentAudioDigest,
					   sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[PdmAudioIn.ReferenceAudioDigest.Index]));
				PdmAudioIn.ReferenceAudioDigest.Index++;
				ChangeGreenLed(TLedMode::FastFastFlash);
				if (PdmAudioIn.ReferenceAudioDigest.Index
					>= sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames) / sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[0])) {
					PdmAudioIn.StoreReferenceAudio = false;
					PdmAudioIn.ReferenceAudioDigest.Index = 0;
					WriteAudioDigest0(&PdmAudioIn.ReferenceAudioDigest.Fragment);
					ChangeGreenLed(TLedMode::Off);
				}
			} else if (PdmAudioIn.MaxBinValue > 60000) {
				AudioMatching();
			} else if (PdmAudioIn.Equability > 0) {
				PdmAudioIn.Equability--;
			}
		}
	}
}

static void AudioMatching() {
	if (test) {
		test = false;
	}
	int equability = 0;
	for (size_t i = 0; i < sizeof(PdmAudioIn.CurrentAudioDigest.FftBins) / sizeof(PdmAudioIn.CurrentAudioDigest.FftBins[0]); i++) {
		PTAudioDigest referenceFrame = &PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[PdmAudioIn.ReferenceAudioDigest.Index];
		uint8_t reference = referenceFrame->FftBins[i];
		uint8_t sampleCurrent = PdmAudioIn.CurrentAudioDigest.FftBins[i];
		uint8_t samplePrior = i > 0 //
								  ? PdmAudioIn.CurrentAudioDigest.FftBins[i - 1]
								  : 0;
		uint8_t sampleNext = i < (sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames) / sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[0])) - 1 //
								 ? PdmAudioIn.CurrentAudioDigest.FftBins[i + 1]
								 : 0;

		if (reference > 0) {
			if (Match(reference, sampleCurrent)						   //
				|| (samplePrior != 0 && Match(reference, samplePrior)) //
				|| (sampleNext != 0 && Match(reference, sampleNext))) {
				equability++;
			} else {
				equability--;
			}
		}
	}

	PdmAudioIn.Equability += equability;

	if (PdmAudioIn.Equability >= 1) {
		if (PdmAudioIn.ReferenceAudioDigest.Index > 2) {
			ChangeBlueLed(TLedMode::Pulse);
		}
		PdmAudioIn.ReferenceAudioDigest.Index++;
		if (PdmAudioIn.ReferenceAudioDigest.Index
			>= sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames) / sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[0])) {
			PdmAudioIn.ReferenceAudioDigest.Index = 0;
		}
	} else {
		PdmAudioIn.ReferenceAudioDigest.Index = 0;
		PdmAudioIn.Equability = 0;
	}
}

static bool Match(uint8_t reference, uint8_t analysed) {
	int diff = reference - analysed;

	if (diff > 0) {
		if (diff < reference - (reference / 4)) {
			return true;
		}
	} else if (diff < 0) {
		if (-diff < analysed - (analysed / 4)) {
			return true;
		}
	} else {
		return true;
	}
	return 0;
}