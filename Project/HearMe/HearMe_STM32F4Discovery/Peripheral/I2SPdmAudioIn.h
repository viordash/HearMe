#pragma once

#include "pdm_filter.h"

#define INTERNAL_BUFF_SIZE (64)

typedef struct {
	uint8_t R;
	uint8_t F;
	uint8_t I;
} TAnalysisAudioSample, *PTAnalysisAudioSample;

typedef struct {
	TAnalysisAudioSample Samples[15];
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

	float32_t DecodedBuffer[(INTERNAL_BUFF_SIZE / 4) * 128];
	uint32_t DecodedBufferSize = 0;

	float32_t FftOutput[(sizeof(DecodedBuffer) / sizeof(DecodedBuffer[0])) / 4];

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
