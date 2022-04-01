
#include "Board.h"
#include "FftAnalyze.h"
#include "arm_const_structs.h"

#define TEST_LENGTH_SAMPLES 2048

/* -------------------------------------------------------------------
 * External Input and Output buffer Declarations for FFT Bin Example
 * ------------------------------------------------------------------- */
// extern float32_t testInput_f32_10khz[TEST_LENGTH_SAMPLES];
// static float32_t testOutput[TEST_LENGTH_SAMPLES / 2];

/* ------------------------------------------------------------------
 * Global variables for FFT Bin Example
 * ------------------------------------------------------------------- */
// uint32_t fftSize = 1024;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
arm_cfft_instance_f32 varInstCfftF32;

/* Reference index at which max energy of bin ocuurs */
// uint32_t refIndex = 213, testIndex = 0;

void InitFft(uint32_t dataSize) {
	arm_cfft_init_f32(&varInstCfftF32, dataSize / 2);
}

void FftAnalyze(float32_t *input, uint32_t inputSize, float32_t *output) {
	arm_cfft_f32(&varInstCfftF32, input, ifftFlag, doBitReverse); /* Process the data through the CFFT/CIFFT module */
	arm_cmplx_mag_f32(input, output, inputSize / 2); /* Process the data through the Complex Magnitude Module for calculating the magnitude at each bin */

	//	arm_max_f32(output, inputSize, &maxValue, &testIndex);/* Calculates maxValue and returns corresponding BIN value */
}

// int32_t Analyze() {
//
//	arm_status status;
//	float32_t maxValue;
//
//	status = ARM_MATH_SUCCESS;
//
//	status = arm_cfft_init_f32(&varInstCfftF32, fftSize);
//
//	/* Process the data through the CFFT/CIFFT module */
//	arm_cfft_f32(&varInstCfftF32, testInput_f32_10khz, ifftFlag, doBitReverse);
//
//	/* Process the data through the Complex Magnitude Module for
//	calculating the magnitude at each bin */
//	arm_cmplx_mag_f32(testInput_f32_10khz, testOutput, fftSize);
//
//	/* Calculates maxValue and returns corresponding BIN value */
//	arm_max_f32(testOutput, fftSize, &maxValue, &testIndex);
//
//	status = (testIndex != refIndex) ? ARM_MATH_TEST_FAILURE : ARM_MATH_SUCCESS;
//
//	if (status != ARM_MATH_SUCCESS) {
//
//		while (1)
//			; /* main function does not return */
//
//	} else {
//
//		while (1)
//			; /* main function does not return */
//	}
//}
