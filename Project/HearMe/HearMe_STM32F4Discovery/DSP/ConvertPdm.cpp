
#include "Board.h"
#include "ConvertPdm.h"
#include "ConvertPdmLookup.h"
#include "I2SAudioOut.h"

#define SAMPLES 16 * 8

static uint8_t PDM_temp[992];
static float32_t PCM_buffer[SAMPLES];
static uint32_t counter;

static void cost_calc(float *PCM_buff, uint32_t *count);
static void PCM_post_processing(float *PCM_buff, int size);

void PutPdmData(uint16_t *data128) {
	ConvertPdm2Pcm(data128, &PCM_buffer[counter]);
	counter += 16;

	//	if (counter == 512) {
	//		cost_calc(PCM_buffer, &counter);
	//	}
	//	if (counter == 1024) {
	//		cost_calc(PCM_buffer, &counter);
	//	}
}

bool PdmConverted(float32_t *outBuffer, int outBufferBytes) {
	if (counter < SAMPLES) {
		return false;
	}
	//	PCM_post_processing(PCM_buffer, counter);
	counter = 0;
	memcpy(outBuffer, PCM_buffer, outBufferBytes);

	return true;
}

void ConvertPdm2Pcm(uint16_t *pdmData128, float32_t *pcmOutBuffer16) {
	int i = 0, j, k, l, m;
	uint8_t *var;
	float temp, temp1, temp2, temp3, temp4;

	var = &PDM_temp[480];
	while (i < 512) {
		*var++ = (uint8_t)((pdmData128[i >> 2] & 0xF000) >> 12);
		*var++ = (uint8_t)((pdmData128[i >> 2] & 0x0F00) >> 8);
		*var++ = (uint8_t)((pdmData128[i >> 2] & 0x00F0) >> 4);
		*var++ = (uint8_t)(pdmData128[i >> 2] & 0x000F);
		i += 4;
	}
	i = 0;
	while (i < 16) {
		var = &PDM_temp[i * 32];
		temp = -4; //-4 is to remove dc
		j = 0;
		k = 1;
		l = 2;
		m = 3;
		while (j < 256) {
			temp1 = LPF_precalc[j][var[j]];
			temp2 = LPF_precalc[k][var[k]];
			temp3 = LPF_precalc[l][var[l]];
			temp4 = LPF_precalc[m][var[m]];
			j += 4;
			k = j + 1;
			l = j + 2;
			m = j + 3;
			temp = temp + temp1 + temp2 + temp3 + temp4;
		}
		pdmData128[i] = temp;
		i++;
	}
	memcpy(&PDM_temp[0], &PDM_temp[512], 480);
}

// static void cost_calc(float *PCM_buff, uint32_t *count) {
//	float cost = 0, temp;
//	if (*count == 512) {
//		for (int i = 0; i < 512; i++) {
//			abzf(temp, PCM_buff[i]);
//			cost += temp;
//		}
//		if (cost < 20) {
//			*count = 0;
//		}
//	} else if (*count == 1024) {
//		for (int i = 512; i < 1024; i++) {
//			abzf(temp, PCM_buff[i]);
//			cost += temp;
//		}
//		if (cost < 50) {
//			if (cost > 20) {
//				memcpy(PCM_buff, &PCM_buff[512], 2048);
//				*count = 512;
//			} else {
//				*count = 0;
//			}
//		}
//	}
//}

static void PCM_post_processing(float *PCM_buff, int size) {
	float avg = 0, max = 0, temp;
	for (int i = 0; i < size; i++) {
		avg += PCM_buff[i];
	}
	avg /= size;
	for (int i = 0; i < size; i++) {
		PCM_buff[i] = PCM_buff[i] - avg;
		abzf(temp, PCM_buff[i]);
		if (max < temp) {
			max = temp;
		}
	}
	max /= 0.95f;
	for (int i = 0; i < size; i++) {
		PCM_buff[i] = PCM_buff[i] / max;
	}
}