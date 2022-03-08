#pragma once

#include "pdm_filter.h"

/* PDM buffer input size */
#define INTERNAL_BUFF_SIZE 64

typedef struct {
	PDMFilter_InitStruct Filter;
	int InternalBufferIndex;
	uint16_t InternalBuffer0[INTERNAL_BUFF_SIZE];
	uint16_t InternalBuffer1[INTERNAL_BUFF_SIZE];
	uint32_t InternalBufferSize = 0;
	QueueHandle_t ReadyDataQueue;

	uint32_t StereoBufferSize = 0;
	int16_t StereoBuffer[(INTERNAL_BUFF_SIZE / 4) * 2 * 8];
} TPdmAudioIn, *PTPdmAudioIn;

extern TPdmAudioIn PdmAudioIn;

void InitPdmAudioIn();
void StartPdmAudioIn();
void StopPdmAudioIn();
