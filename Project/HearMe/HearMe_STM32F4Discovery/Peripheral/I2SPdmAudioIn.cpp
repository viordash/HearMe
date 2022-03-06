
#include "Board.h"
#include "I2SPdmAudioIn.h"
#include "TimeHelper.h"
#include "pdm_filter.h"
#include "I2SAudioOut.h"

/* PDM buffer input size */
#define INTERNAL_BUFF_SIZE 64

typedef struct {
	PDMFilter_InitStruct Filter;
	uint16_t InternalBuffer[INTERNAL_BUFF_SIZE];
	uint32_t InternalBufferSize = 0;
	int16_t pAudioRecBuf[INTERNAL_BUFF_SIZE / 4];
	uint32_t StereoBufferSize = 0;
	int16_t StereoBuffer[(sizeof(pAudioRecBuf) / sizeof(pAudioRecBuf[0])) * 2 * 8];
	int16_t StereoBuffer1[(sizeof(pAudioRecBuf) / sizeof(pAudioRecBuf[0])) * 2 * 8];
} TPdmAudioIn, *PTPdmAudioIn;

TPdmAudioIn PdmAudioIn;

static void NVIC_Init(void);
static void SPI_Init(uint32_t Freq);

void InitPdmAudioIn() {
	memset(&PdmAudioIn, 0, sizeof(PdmAudioIn));

	__CRC_CLK_ENABLE(); /* Enable CRC module */

	/* Filter LP & HP Init */
	PdmAudioIn.Filter.LP_HZ = 8000;
	PdmAudioIn.Filter.HP_HZ = 100;
	PdmAudioIn.Filter.Fs = 16000;
	PdmAudioIn.Filter.Out_MicChannels = 1;
	PdmAudioIn.Filter.In_MicChannels = 1;

	PDM_Filter_Init((PDMFilter_InitStruct *)&PdmAudioIn.Filter);

	NVIC_Init();

	SPI_Init(PdmAudioIn.Filter.Fs * 2);
}

void StartPdmAudioIn() {
	I2S_HandleTypeDef hi2s2;
	hi2s2.Instance = SPI2;
	StartAudioOut((uint16_t *)PdmAudioIn.StereoBuffer1, sizeof(PdmAudioIn.StereoBuffer1), 80, false);
	__HAL_I2S_ENABLE_IT(&hi2s2, (I2S_IT_RXNE /*| I2S_IT_ERR*/)); /* Enable RXNE and ERR interrupt */
	__HAL_I2S_ENABLE(&hi2s2);
}

void StopPdmAudioIn() {
	I2S_HandleTypeDef hi2s2;
	hi2s2.Instance = SPI2;
	__HAL_I2S_DISABLE(&hi2s2);
	__HAL_I2S_DISABLE_IT(&hi2s2, (I2S_IT_RXNE /*| I2S_IT_ERR*/)); /* Disable RXNE and ERR interrupt */
}

static void NVIC_Init(void) {
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
	HAL_NVIC_SetPriority(SPI2_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(SPI2_IRQn);
}

static void SPI_Init(uint32_t audioFreq) {
	I2S_HandleTypeDef hi2s = {};

	__SPI2_CLK_ENABLE();

	hi2s.Instance = SPI2;
	hi2s.Init.AudioFreq = audioFreq;
	hi2s.Init.Standard = I2S_STANDARD_LSB;
	hi2s.Init.DataFormat = I2S_DATAFORMAT_16B;
	hi2s.Init.CPOL = I2S_CPOL_HIGH;
	hi2s.Init.Mode = I2S_MODE_MASTER_RX;
	hi2s.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
	HAL_I2S_Init(&hi2s);
}

extern "C" void SPI2_IRQHandler() {
	I2S_HandleTypeDef hi2s2;
	hi2s2.Instance = SPI2;
	I2S_HandleTypeDef *hi2s = &hi2s2;
	__IO uint32_t i2ssr = hi2s->Instance->SR;
	if (((i2ssr & I2S_FLAG_RXNE) == I2S_FLAG_RXNE) && (__HAL_I2S_GET_IT_SOURCE(hi2s, I2S_IT_RXNE) != RESET)) {

		uint16_t app = ((uint16_t)hi2s->Instance->DR);
		PdmAudioIn.InternalBuffer[PdmAudioIn.InternalBufferSize++] = __htons(app);

		TogglePortPin(TEST2_PORT, TEST2_PIN);
		/* Check to prevent overflow condition */
		if (PdmAudioIn.InternalBufferSize >= INTERNAL_BUFF_SIZE) {
			//			TogglePortPin(TEST1_PORT, TEST1_PIN);
			PdmAudioIn.InternalBufferSize = 0;

			uint16_t volume = 100;
			SetPortPin(TEST1_PORT, TEST1_PIN);
			PDM_Filter_64_LSB((uint8_t *)PdmAudioIn.InternalBuffer, (uint16_t *)PdmAudioIn.pAudioRecBuf, volume, (PDMFilter_InitStruct *)&PdmAudioIn.Filter);
			ResetPortPin(TEST1_PORT, TEST1_PIN);

			for (size_t i = 0; i < sizeof(PdmAudioIn.pAudioRecBuf) / sizeof(PdmAudioIn.pAudioRecBuf[0]); i++) {
				int16_t val = PdmAudioIn.pAudioRecBuf[i];
				if (val > 32000) {
					val -= 100;
					//					SetPortPin(TEST1_PORT, TEST1_PIN);
				} else if (val < -32000) {
					val += 100;
					//					ResetPortPin(TEST1_PORT, TEST1_PIN);
				}

				PdmAudioIn.StereoBuffer[PdmAudioIn.StereoBufferSize++] = (val);
				PdmAudioIn.StereoBuffer[PdmAudioIn.StereoBufferSize++] = (val);
			}
			if (PdmAudioIn.StereoBufferSize >= sizeof(PdmAudioIn.StereoBuffer) / sizeof(PdmAudioIn.StereoBuffer[0])) {
				memcpy(PdmAudioIn.StereoBuffer1, PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer1));
				PlayAudioOut((uint16_t *)PdmAudioIn.StereoBuffer1, sizeof(PdmAudioIn.StereoBuffer1));
				PdmAudioIn.StereoBufferSize = 0;
			}
		}
	}

	//	/* I2S Overrun error interrupt occurred -------------------------------------*/
	//	if (((i2ssr & I2S_FLAG_OVR) == I2S_FLAG_OVR) && (__HAL_I2S_GET_IT_SOURCE(hi2s, I2S_IT_ERR) != RESET)) {
	//
	//		//		__HAL_I2S_DISABLE_IT(hi2s, (I2S_IT_RXNE | I2S_IT_ERR)); /* Disable RXNE and ERR interrupt */
	//		__HAL_I2S_CLEAR_OVRFLAG(hi2s); /* Clear Overrun flag */
	//		TogglePortPin(TEST1_PORT, TEST1_PIN);
	//	}
}