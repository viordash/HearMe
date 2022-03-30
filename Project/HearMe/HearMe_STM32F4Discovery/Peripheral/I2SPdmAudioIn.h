#pragma once

#include "pdm_filter.h"

#define INTERNAL_BUFF_SIZE (64)

typedef struct {
	uint8_t R;
	uint8_t F;
	uint8_t I;
} TAnalysisAudioSample, *PTAnalysisAudioSample;

typedef struct {
	TAnalysisAudioSample Samples[50];
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
	QueueHandle_t ReadyDataQueue;
	uint16_t MicLevel;

	int16_t DecodedBuffer[(INTERNAL_BUFF_SIZE / 4) * 40];
	uint32_t DecodedBufferSize = 0;

	uint32_t Amplitude;
	int16_t PrevDiff;
	int16_t PrevValue;
	int8_t PrevValueVectorized;
	int8_t PrevVectorized;
	int8_t Vectorized[sizeof(DecodedBuffer) / sizeof(DecodedBuffer[0])];

//	int16_t StereoBuffer[(sizeof(DecodedBuffer) / sizeof(DecodedBuffer[0])) * 2];

	TAudioDigest CurrentAudioDigest;
	TReferenceAudioDigest ReferenceAudioDigest;
	bool RequestToStoreReferenceAudio;
	bool StoreReferenceAudio;
	uint16_t Equability;

	PDMFilter_InitStruct Filter;
} TPdmAudioIn, *PTPdmAudioIn;

extern TPdmAudioIn PdmAudioIn;

void InitPdmAudioIn();
void StartPdmAudioIn();
void StopPdmAudioIn();
