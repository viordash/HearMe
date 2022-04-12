
#include "Board.h"
#include "ConvertPdm.h"
#include "ConvertPdmLookup.h"
#include "I2SAudioOut.h"

static uint8_t PDM_temp[992];

static void cost_calc(float *PCM_buff, uint32_t *count);

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
		pcmOutBuffer16[i] = temp;
		i++;
	}
	memcpy(&PDM_temp[0], &PDM_temp[512], 480);
}

void PCM_post_processing(float *PCM_buff, int size) {
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