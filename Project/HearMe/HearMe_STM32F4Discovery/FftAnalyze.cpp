
#include "Board.h"
#include "FftAnalyze.h"
#include "arm_const_structs.h"

uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
arm_cfft_instance_f32 varInstCfftF32;

extern float32_t testInput_f32_10khz[2048];
extern float32_t testInput_f32_1kHz_15kHz[320];

#define SAMPLES_COUNT 256
float32_t debugInputBuffer[SAMPLES_COUNT];

float32_t inputBuffer[SAMPLES_COUNT * 2];
static float32_t testOutput[SAMPLES_COUNT / 2];

void InitFft(uint32_t dataSize) {
	arm_cfft_init_f32(&varInstCfftF32, dataSize / 2);

	//	//	memcpy(inputBuffer, testInput_f32_10khz, sizeof(inputBuffer));
	//
	//	for (size_t i = 0; i < SAMPLES_COUNT; i++) {
	//		float32_t val = testInput_f32_1kHz_15kHz[i];
	//		inputBuffer[i * 2] = val;
	//		inputBuffer[(i * 2) + 1] = 0;
	//	}
	//
	//	for (size_t i = 0; i < SAMPLES_COUNT; i++) {
	//		float32_t val = inputBuffer[i * 2];
	//		debugInputBuffer[i] = val;
	//	}
	//
	//	arm_cfft_init_f32(&varInstCfftF32, SAMPLES_COUNT);
	//	arm_cfft_f32(&varInstCfftF32, inputBuffer, ifftFlag, doBitReverse); /* Process the data through the CFFT/CIFFT module */
	//	arm_cmplx_mag_f32(inputBuffer, testOutput, SAMPLES_COUNT / 2);
}

void FftAnalyze(float32_t *input, float32_t *output, uint32_t outputSize) {
	arm_cfft_f32(&varInstCfftF32, input, ifftFlag, doBitReverse); /* Process the data through the CFFT/CIFFT module */
	arm_cmplx_mag_f32(input, output, outputSize); /* Process the data through the Complex Magnitude Module for calculating the magnitude at each bin */
}
