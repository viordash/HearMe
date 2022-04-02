
#include "Board.h"
#include "I2SPdmAudioIn.h"
#include "TimeHelper.h"
#include "I2SAudioOut.h"

#define SampleRate 16000

TPdmAudioIn PdmAudioIn;

static void NVIC_Init(void);
static void SPI_Init(uint32_t Freq);

void InitPdmAudioIn() {
	memset(&PdmAudioIn, 0, sizeof(PdmAudioIn));

	__CRC_CLK_ENABLE(); /* Enable CRC module */

	PdmAudioIn.ReadyDataQueue = xQueueCreate(2, sizeof(uint16_t *));

	PdmAudioIn.Filter.LP_HZ = 4000;
	PdmAudioIn.Filter.HP_HZ = 400;
	PdmAudioIn.Filter.Fs = SampleRate;
	PdmAudioIn.Filter.Out_MicChannels = 1;
	PdmAudioIn.Filter.In_MicChannels = 1;
	PdmAudioIn.MicLevel = 250;

	PDM_Filter_Init((PDMFilter_InitStruct *)&PdmAudioIn.Filter);
	
	NVIC_Init();

	SPI_Init(SampleRate * 2);
}

void StartPdmAudioIn() {
	I2S_HandleTypeDef hi2s2;
	hi2s2.Instance = SPI2;
//	StartAudioOut((uint16_t *)PdmAudioIn.StereoBuffer, sizeof(PdmAudioIn.StereoBuffer), 80, false);
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
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	HAL_NVIC_SetPriority(SPI2_IRQn, 6, 0);
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

		uint16_t *pBuffer;
		switch (PdmAudioIn.InternalBufferIndex) {
			case 1:
				pBuffer = PdmAudioIn.InternalBuffer1;
				break;
			default:
				pBuffer = PdmAudioIn.InternalBuffer0;
				break;
		}

		pBuffer[PdmAudioIn.InternalBufferSize++] = __htons(app);
		if (PdmAudioIn.InternalBufferSize >= INTERNAL_BUFF_SIZE) {
			PdmAudioIn.InternalBufferSize = 0;
			PdmAudioIn.InternalBufferIndex = (PdmAudioIn.InternalBufferIndex + 1) & 0x01;
			xQueueSendFromISR(PdmAudioIn.ReadyDataQueue, (void *)&pBuffer, NULL);
		}
	}
}