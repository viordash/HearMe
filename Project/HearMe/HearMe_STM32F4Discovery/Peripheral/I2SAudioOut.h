#pragma once

typedef enum { //
	CODEC_PDWN_HW = 1,
	CODEC_PDWN_SW = 2
} TCodecPowerdown;

typedef enum { //
	AUDIO_MUTE_ON = 1,
	AUDIO_MUTE_OFF = 0
} TCodecAudioMute;

void InitAudioOut();
void StartAudioOut(uint16_t *pData, uint32_t size, uint8_t volume, bool circularPlay);
void StopAudioOut(TCodecPowerdown powerdown);
