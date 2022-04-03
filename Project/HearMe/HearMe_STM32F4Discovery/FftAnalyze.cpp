
#include "Board.h"
#include "FftAnalyze.h"
#include "arm_const_structs.h"

uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
arm_cfft_instance_f32 varInstCfftF32;

void InitFft(uint32_t dataSize) {
	arm_cfft_init_f32(&varInstCfftF32, dataSize / 2);
}

void FftAnalyze(float32_t *input, float32_t *magnitude, uint32_t magnitudeSize) {
	arm_cfft_f32(&varInstCfftF32, input, ifftFlag, doBitReverse); /* Process the data through the CFFT/CIFFT module */
	arm_cmplx_mag_f32(input, magnitude, magnitudeSize); /* Process the data through the Complex Magnitude Module for calculating the magnitude at each bin */
}

bool GetBinBorders(const float32_t *magnitude, uint32_t magnitudeSize, uint32_t binIndex, uint32_t *startIndex, uint32_t *endIndex) {
	*startIndex = 0;
	float32_t binVal = magnitude[binIndex];

	float32_t val = binVal;
	for (int i = binIndex - 1; i > 0; i--) {
		if (magnitude[i] == 0 || magnitude[i] > val) {
			*startIndex = i + 1;
			break;
		}
		val = magnitude[i];
	}

	*endIndex = magnitudeSize - 1;
	val = binVal;
	for (int i = binIndex + 1; i < magnitudeSize; i++) {
		if (magnitude[i] > val) {
			*endIndex = i - 1;
			break;
		}
		val = magnitude[i];
	}
	return *startIndex > 0 || *endIndex < magnitudeSize;
}

void GetMaxMagnitude(const float32_t *pSrc, uint32_t blockSize, float32_t *pResult, uint32_t *pIndex) {
	arm_max_f32(pSrc, blockSize, pResult, pIndex);
}