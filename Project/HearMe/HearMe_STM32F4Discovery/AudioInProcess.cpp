
#include "Board.h"
#include "AudioInProcess.h"
#include "I2SAudioOut.h"
#include "AudioSample.h"
#include "FlashMemStorage.h"
#include "FftAnalyze.h"
#include "Leds.h"

TAudioInProcess AudioInProcess;

static void ExtractAudioDigest();
static void NormalizeLevels(PTAudioDigest pAudioDigest);
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

			for (size_t i = 0; i < (sizeof(PdmAudioIn.DecodedBuffer0) / sizeof(PdmAudioIn.DecodedBuffer0[0])) / 2; i++) {
				int16_t val = decodedBuffer[i * 2];
				PdmAudioIn.StereoBuffer[i * 2] = val;
				PdmAudioIn.StereoBuffer[(i * 2) + 1] = val;
			}
			PlayAudioOut((uint16_t *)PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer));

			SetPortPin(TEST2_PORT, TEST2_PIN);
			FftAnalyze(decodedBuffer, PdmAudioIn.FftMagnitude, sizeof(PdmAudioIn.FftMagnitude) / sizeof(PdmAudioIn.FftMagnitude[0]));
			ExtractAudioDigest();
			ResetPortPin(TEST2_PORT, TEST2_PIN);

			if (PdmAudioIn.RequestToStoreReferenceAudio && PdmAudioIn.MaxBinValue > 200000) {
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
			} else if (PdmAudioIn.MaxBinValue > 30000) {
				SetPortPin(TEST1_PORT, TEST1_PIN);
				AudioMatching();
				ResetPortPin(TEST1_PORT, TEST1_PIN);
			} else if (PdmAudioIn.Equability > 0) {
				PdmAudioIn.Equability -= PdmAudioIn.Equability / 5;
			}
		}
	}
}

static void ExtractAudioDigest() {
	const uint32_t magnitudeSize = sizeof(PdmAudioIn.FftMagnitude) / sizeof(PdmAudioIn.FftMagnitude[0]);
	float32_t maxBinValue;
	float32_t binValue;
	uint32_t binIndex;

	memset(PdmAudioIn.CurrentAudioDigest.FftBins, 0, sizeof(PdmAudioIn.CurrentAudioDigest.FftBins));
	GetMaxMagnitude(PdmAudioIn.FftMagnitude, magnitudeSize, &maxBinValue, &binIndex);
	binValue = maxBinValue;
	const float32_t cutOff = maxBinValue / 8;
	int binCount = 10;

	while (binValue > cutOff && binCount-- > 0) {
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
}

static uint8_t GetMaxValue(PTAudioDigest pAudioDigest) {
	uint8_t out = 0;
	for (size_t i = 0; i < sizeof(pAudioDigest->FftBins) / sizeof(pAudioDigest->FftBins[0]); i++) {
		if (out < pAudioDigest->FftBins[i]) {
			out = pAudioDigest->FftBins[i];
			if (out == UINT8_MAX) {
				break;
			}
		}
	}
	return out;
}

static bool PrepareAudioMatching(PTAudioDigest referenceFrame) {
	uint32_t significantWeight = 0;
	uint32_t equalsWeight = 0;

	for (size_t i = 0; i < sizeof(TAudioDigest::FftBins) / sizeof(TAudioDigest::FftBins[0]); i++) {
		uint8_t reference = referenceFrame->FftBins[i];
		if (reference == 0) {
			PdmAudioIn.CurrentAudioDigest.FftBins[i] = 0;
			continue;
		}
		significantWeight += reference;

		uint8_t sampleCurrent = PdmAudioIn.CurrentAudioDigest.FftBins[i];
		if (sampleCurrent > 0) {
			equalsWeight += sampleCurrent;
			continue;
		}

		uint8_t samplePrior = i > 0 //
								  ? PdmAudioIn.CurrentAudioDigest.FftBins[i - 1]
								  : 0;
		if (samplePrior > 0 && referenceFrame->FftBins[i - 1] == 0) {
			PdmAudioIn.CurrentAudioDigest.FftBins[i] = samplePrior;
			PdmAudioIn.CurrentAudioDigest.FftBins[i - 1] = 0;
			equalsWeight += samplePrior;
			continue;
		}

		uint8_t sampleNext = i < (sizeof(TAudioDigest::FftBins) / sizeof(TAudioDigest::FftBins[0])) - 1 //
								 ? PdmAudioIn.CurrentAudioDigest.FftBins[i + 1]
								 : 0;
		if (sampleNext > 0 && referenceFrame->FftBins[i + 1] == 0) {
			PdmAudioIn.CurrentAudioDigest.FftBins[i] = sampleNext;
			PdmAudioIn.CurrentAudioDigest.FftBins[i + 1] = 0;
			equalsWeight += sampleNext;
			continue;
		}

		uint8_t samplePriorPrior = i > 1 //
									   ? PdmAudioIn.CurrentAudioDigest.FftBins[i - 2]
									   : 0;
		if (samplePriorPrior > 0 && referenceFrame->FftBins[i - 2] == 0) {
			PdmAudioIn.CurrentAudioDigest.FftBins[i] = samplePriorPrior;
			PdmAudioIn.CurrentAudioDigest.FftBins[i - 2] = 0;
			equalsWeight += samplePriorPrior;
			continue;
		}

		uint8_t sampleNextNext = i < (sizeof(TAudioDigest::FftBins) / sizeof(TAudioDigest::FftBins[0])) - 2 //
									 ? PdmAudioIn.CurrentAudioDigest.FftBins[i + 2]
									 : 0;
		if (sampleNextNext > 0 && referenceFrame->FftBins[i + 2] == 0) {
			PdmAudioIn.CurrentAudioDigest.FftBins[i] = sampleNextNext;
			PdmAudioIn.CurrentAudioDigest.FftBins[i + 2] = 0;
			equalsWeight += sampleNextNext;
			continue;
		}
		PdmAudioIn.CurrentAudioDigest.FftBins[i] = 0;
		referenceFrame->FftBins[i] = 0;
	}
	const uint32_t significantCount_70perc = significantWeight - (significantWeight / 3);
	return equalsWeight >= significantCount_70perc;
}

TAudioDigest localReferenceFrame;

static void AudioMatching() {
	if (test) {
		test = false;
	}

	int equability = 0;

	while (true) {
		int significantCount = 0;
		localReferenceFrame = PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[PdmAudioIn.ReferenceAudioDigest.Index];

		if (!PrepareAudioMatching(&localReferenceFrame)) {
			if (++PdmAudioIn.ReferenceAudioDigest.Index
				> (sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames) / sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[0]) / 3)) {
				PdmAudioIn.ReferenceAudioDigest.Index = 0;
				PdmAudioIn.Equability = 0;
				return;
			}
			continue;
		}
		NormalizeLevels(&localReferenceFrame);
		NormalizeLevels(&PdmAudioIn.CurrentAudioDigest);

		for (size_t i = 0; i < sizeof(TAudioDigest::FftBins) / sizeof(TAudioDigest::FftBins[0]); i++) {
			uint8_t reference = localReferenceFrame.FftBins[i];
			if (reference == 0) {
				continue;
			}
			significantCount++;
			uint8_t sampleCurrent = PdmAudioIn.CurrentAudioDigest.FftBins[i];
			//			if (Match(reference, sampleCurrent)) {
			if (sampleCurrent > 0) {
				equability++;
				continue;
			}
		}

		const int significantCount_70perc = significantCount - (significantCount / 3);
		if (equability < significantCount_70perc) {
			if (++PdmAudioIn.ReferenceAudioDigest.Index
				> (sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames) / sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[0]) / 3)) {
				PdmAudioIn.ReferenceAudioDigest.Index = 0;
				PdmAudioIn.Equability = 0;
				return;
			}
			continue;
		}
		break;
	}

	PdmAudioIn.Equability += equability;
	if (PdmAudioIn.Equability > 30) {
		ChangeBlueLed(TLedMode::Pulse);
	}
	PdmAudioIn.ReferenceAudioDigest.Index++;
	if (PdmAudioIn.ReferenceAudioDigest.Index
		>= sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames) / sizeof(PdmAudioIn.ReferenceAudioDigest.Fragment.Frames[0])) {
		PdmAudioIn.ReferenceAudioDigest.Index = 0;
	}
}

static bool Match(uint8_t reference, uint8_t analysed) {
	int diff = reference - analysed;

	if (diff > 0) {
		if (diff < reference - (reference / 3)) {
			return true;
		}
	} else if (diff < 0) {
		if (-diff < analysed - (analysed / 3)) {
			return true;
		}
	} else {
		return true;
	}
	return 0;
}

static void NormalizeLevels(PTAudioDigest pAudioDigest) {
	const uint32_t count = sizeof(pAudioDigest->FftBins) / sizeof(pAudioDigest->FftBins[0]);
	uint8_t maxValue = GetMaxValue(pAudioDigest);
	float32_t factor = 255.0 / maxValue;

	for (size_t i = 0; i < sizeof(pAudioDigest->FftBins) / sizeof(pAudioDigest->FftBins[0]); i++) {
		float32_t val = pAudioDigest->FftBins[i];
		pAudioDigest->FftBins[i] = val * factor;
	}
}