#pragma once

#include "Board.h"
#include "pdm_filter.h"

#define INTERNAL_BUFF_SIZE (128)

#define DecodedBufferCount ((INTERNAL_BUFF_SIZE / 4) * 64)
#define FftOutSamplesCount (DecodedBufferCount / 4)
#define FftOutSamplesCountForFilteredPass (FftOutSamplesCount / ((SampleRate / 2) / FilterLowPassHz))

typedef struct {
	uint8_t FftBins[FftOutSamplesCountForFilteredPass];
} TAudioDigest, *PTAudioDigest;

typedef struct {
	TAudioDigest Frames[50];
} TAudioFragmentAnalysis, *PTAudioFragmentAnalysis;

typedef struct {
	TAudioFragmentAnalysis Fragment;
	int Index;
	int MatchedFramesCount;
} TReferenceAudioDigest, *PTReferenceAudioDigest;

typedef struct {
	int InternalBufferIndex;
	uint16_t InternalBuffer0[INTERNAL_BUFF_SIZE];
	uint16_t InternalBuffer1[INTERNAL_BUFF_SIZE];
	uint32_t InternalBufferSize = 0;

	TaskHandle_t TaskHandle;
	QueueHandle_t ReadyPdmDataQueue;
	QueueHandle_t ReadyDecodedDataQueue;
	uint16_t MicLevel;

	int DecodedBufferIndex;
	uint32_t DecodedBufferSize = 0;
	float32_t DecodedBuffer0[DecodedBufferCount];
	float32_t DecodedBuffer1[DecodedBufferCount];

	float32_t FftMagnitude[FftOutSamplesCountForFilteredPass];

	float32_t MaxBinValue;
	TAudioDigest CurrentAudioDigest;
	TReferenceAudioDigest ReferenceAudioDigest;
	bool RequestToStoreReferenceAudio;
	bool StoreReferenceAudio;
	int16_t Equability;

	PDMFilter_InitStruct Filter;

	int16_t StereoBuffer[(sizeof(DecodedBuffer0) / sizeof(DecodedBuffer0[0])) * 1];
} TPdmAudioIn, *PTPdmAudioIn;

extern TPdmAudioIn PdmAudioIn;

void InitPdmAudioIn();
void StartPdmAudioIn();
void StopPdmAudioIn();

void TaskPdmAudioDecode(void *arg);