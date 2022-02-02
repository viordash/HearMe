
#include "Board.h"
#include "I2SPdmAudioIn.h"
#include "TimeHelper.h"
#include "pdm_filter.h"

typedef struct {
	PDMFilter_InitStruct Filter;
} TPdmAudioIn, *PTPdmAudioIn;

TPdmAudioIn PdmAudioIn;

void InitPdmAudioIn();
void StartPdmAudioIn();

static void NVIC_Init(void);
static void SPI_Init(uint32_t Freq);

void InitPdmAudioIn() {
	memset(&PdmAudioIn, 0, sizeof(PdmAudioIn));

	/* Enable CRC module */
	RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;

	/* Filter LP & HP Init */
	PdmAudioIn.Filter.LP_HZ = 8000;
	PdmAudioIn.Filter.HP_HZ = 100;
	PdmAudioIn.Filter.Fs = 16000;
	PdmAudioIn.Filter.Out_MicChannels = 1;
	PdmAudioIn.Filter.In_MicChannels = 1;

	PDM_Filter_Init((PDMFilter_InitStruct *)&PdmAudioIn.Filter);

	NVIC_Init();

	SPI_Init(PdmAudioIn.Filter.Fs);
	//
	//	/* Set the local parameters */
	//	AudioRecBitRes = BitRes;
	//	AudioRecChnlNbr = ChnlNbr;
}

void StartPdmAudioIn() {
}

void StopPdmAudioIn() {
}

static void NVIC_Init(void) {
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
	HAL_NVIC_SetPriority(SPI2_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(SPI2_IRQn);
}

static void SPI_Init(uint32_t Freq) {
	I2S_HandleTypeDef hi2s = {};
	SPI_HandleTypeDef hspi = {};

	__SPI2_CLK_ENABLE();

	/* SPI configuration */
	hspi.Instance = SPI2;
	HAL_SPI_DeInit(&hspi);

	hi2s.Instance = SPI2;
	hi2s.Init.AudioFreq = 32000;
	hi2s.Init.Standard = I2S_STANDARD_LSB;
	hi2s.Init.DataFormat = I2S_DATAFORMAT_16B;
	hi2s.Init.CPOL = I2S_CPOL_HIGH;
	hi2s.Init.Mode = I2S_MODE_MASTER_RX;
	hi2s.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
	HAL_I2S_Init(&hi2s);

	/* Enable the Rx buffer not empty interrupt */
	//	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
}
