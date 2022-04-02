
#include "Board.h"
#include "FftAnalyze.h"
#include "arm_const_structs.h"

uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
arm_cfft_instance_f32 varInstCfftF32;

void InitFft(uint32_t dataSize) {
	arm_cfft_init_f32(&varInstCfftF32, dataSize / 2);
}

void FftAnalyze(float32_t *input, float32_t *output, uint32_t outputSize) {
	arm_cfft_f32(&varInstCfftF32, input, ifftFlag, doBitReverse); /* Process the data through the CFFT/CIFFT module */
	arm_cmplx_mag_f32(input, output, outputSize); /* Process the data through the Complex Magnitude Module for calculating the magnitude at each bin */
}
