
#include "Board.h"
#include "I2SAudioOut.h"
#include "TimeHelper.h"
#include "pdm_filter.h"

/* Delay for the Codec to be correctly reset */
#define CODEC_RESET_DELAY 0x4FFF
#define CODEC_FLAG_TIMEOUT ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT ((uint32_t)(300 * CODEC_FLAG_TIMEOUT))
#define CODEC_I2C I2C1
#define CODEC_STANDARD 0x04
#define CODEC_ADDRESS 0x94 /* b00100111 */ /* The 7 bits Codec address (sent through I2C interface) */
#define I2S_STANDARD I2S_Standard_Phillips
#define CODEC_I2S SPI3
#define OUTPUT_DEVICE_AUTO 4
#define VOLUME_CONVERT(x) ((x > 100) ? 100 : ((uint8_t)((x * 255) / 100)))
#define DMA_MAX_SIZE 0xFFFF
#define DMA_MAX(x) (((x) <= DMA_MAX_SIZE) ? (x) : DMA_MAX_SIZE)

#define AUDIO_I2S_DMA_STREAM DMA1_Stream7
#define AUDIO_I2S_DMA_CHANNEL DMA_CHANNEL_0
#define AUDIO_I2S_DMA_IRQ DMA1_Stream7_IRQn

typedef struct {
	__IO uint32_t ISR; /*!< DMA interrupt status register */
	__IO uint32_t Reserved0;
	__IO uint32_t IFCR; /*!< DMA interrupt flag clear register */
} DMA_Base_Registers;

static const uint8_t flagBitshiftOffset[8U] = {0U, 6U, 16U, 22U, 0U, 6U, 16U, 22U};
#define txDmaRegisters ((DMA_Base_Registers *)(((uint32_t)AUDIO_I2S_DMA_STREAM & (uint32_t)(~0x3FFU)) + 4U))
#define txDmaStreamNumber ((((uint32_t)AUDIO_I2S_DMA_STREAM & 0xFFU) - 16U) / 24U)
#define txDmaStreamIndex (flagBitshiftOffset[txDmaStreamNumber])

typedef struct {
	uint32_t TotalSize;
	int32_t RemainingSize;
	uint16_t *CurrentPos;
	uint8_t OutputDev;
	bool CircularPlay;
} TAudioOutCodec, *PTAudioOutCodec;

typedef struct {
	TAudioOutCodec Codec;
	I2C_HandleTypeDef hi2c;
	I2S_HandleTypeDef hi2s;
	DMA_HandleTypeDef hdma;
} TAudioOut, *PTAudioOut;

TAudioOut AudioOut;

void InitAudioOut();
void StartAudioOut();

static void Codec_Reset();
static void Delay(uint32_t nCount);
static void Codec_CtrlInterface_Init();
static bool Codec_WriteRegister(uint8_t registerAddr, uint8_t registerValue);
static void Codec_Init(uint16_t outputDevice, uint32_t audioFreq);
static void Codec_VolumeCtrl(uint8_t volume);
static void Codec_AudioInterface_Init(uint32_t audioFreq);
static void Audio_MAL_Init();
static void Audio_MAL_PauseResume(uint32_t cmd, uint32_t addr);
static void Codec_Mute(TCodecAudioMute cmd);
static void Codec_Stop();
static void DmaTransmit(uint16_t *pData, uint16_t size);

void InitAudioOut() {
	memset(&AudioOut, 0, sizeof(AudioOut));

	Codec_Init(OUTPUT_DEVICE_AUTO, SampleRate);
	Audio_MAL_Init();
}

void StartAudioOut(uint16_t *pData, uint32_t size, uint8_t volume, bool circularPlay) {
	Codec_Mute(AUDIO_MUTE_OFF); /* Unmute the output first */
	Codec_WriteRegister(0x04, AudioOut.Codec.OutputDev);
	Codec_WriteRegister(0x02, 0x9E);		  /* Exit the Power save mode */
	Codec_VolumeCtrl(VOLUME_CONVERT(volume)); /* Set the Master volume */

	AudioOut.Codec.CircularPlay = true;
	PlayAudioOut(pData, size);
}

void PlayAudioOut(uint16_t *pData, uint32_t size) {
	size = size / 2;
	AudioOut.Codec.TotalSize = size;
	AudioOut.Codec.CurrentPos = pData;
	DmaTransmit(pData, DMA_MAX(size));
	AudioOut.Codec.RemainingSize = AudioOut.Codec.TotalSize - DMA_MAX(size);
}

void StopAudioOut() {
	Codec_Stop();
	HAL_I2S_DMAStop(&AudioOut.hi2s);
	AudioOut.Codec.RemainingSize = AudioOut.Codec.TotalSize; /* Update the remaining data number */
}

static void Codec_Reset() {
	WritePortPin(AUDIO_RESET_PORT, AUDIO_RESET_PIN, false); /* Power Down the codec */
	Delay(CODEC_RESET_DELAY);								/* wait for a delay to insure registers erasing */
	WritePortPin(AUDIO_RESET_PORT, AUDIO_RESET_PIN, true);  /* Power on the codec */
}

static void Delay(uint32_t nCount) {
	for (; nCount != 0; nCount--)
		;
}

static void Codec_CtrlInterface_Init() {
	AudioOut.hi2c = {};

	__I2C1_CLK_ENABLE();
	/* CODEC_I2C peripheral configuration */
	AudioOut.hi2c.Instance = CODEC_I2C;
	AudioOut.hi2c.Init.ClockSpeed = 100000;
	AudioOut.hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
	AudioOut.hi2c.Init.OwnAddress1 = 0x33;
	AudioOut.hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	AudioOut.hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	AudioOut.hi2c.Init.OwnAddress2 = 0;
	AudioOut.hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	AudioOut.hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	HAL_I2C_Init(&AudioOut.hi2c);
}

static bool Codec_WriteRegister(uint8_t registerAddr, uint8_t registerValue) {
	uint8_t buffer[2] = {registerAddr, registerValue};

	if (HAL_I2C_Master_Transmit(&AudioOut.hi2c, CODEC_ADDRESS, buffer, sizeof(buffer), CODEC_LONG_TIMEOUT) != HAL_OK) {
		return false;
	}

	uint8_t readedVal;
	if (HAL_I2C_Master_Receive(&AudioOut.hi2c, CODEC_ADDRESS, &readedVal, 1, CODEC_LONG_TIMEOUT) != HAL_OK) {
		return false;
	}
	return registerValue == readedVal ? 0 : 1;
}

static void Codec_Init(uint16_t outputDevice, uint32_t audioFreq) {
	Codec_Reset();				/* Reset the Codec Registers */
	Codec_CtrlInterface_Init(); /* Initialize the Control interface of the Audio Codec */

	Codec_WriteRegister(0x02, 0x01); /* Keep Codec powered OFF */
	AudioOut.Codec.OutputDev = 0xAF;
	Codec_WriteRegister(0x04, AudioOut.Codec.OutputDev); /* SPK always OFF & HP always ON */
	Codec_WriteRegister(0x05, 0x81);					 /* Clock configuration: Auto detection */
	Codec_WriteRegister(0x06, CODEC_STANDARD);			 /* Set the Slave Mode and the audio Standard */
	Codec_WriteRegister(0x02, 0x9E);					 /* Power on the Codec */

	/* Additional configuration for the CODEC. These configurations are done to reduce
		the time needed for the Codec to power off. If these configurations are removed,
		then a long delay should be added between powering off the Codec and switching
		off the I2S peripheral MCLK clock (which is the operating clock for Codec).
		If this delay is not inserted, then the codec will not shut down properly and
		it results in high noise after shut down. */

	Codec_WriteRegister(0x0A, 0x00); /* Disable the analog soft ramp */
	Codec_WriteRegister(0x0E, 0x04); /* Disable the digital soft ramp */
	Codec_WriteRegister(0x27, 0x00); /* Disable the limiter attack level */
	Codec_WriteRegister(0x1F, 0x0F); /* Adjust Bass and Treble levels */
	Codec_WriteRegister(0x1A, 0x0A); /* Adjust PCM volume level */
	Codec_WriteRegister(0x1B, 0x0A);
	Codec_AudioInterface_Init(audioFreq); /* Configure the I2S peripheral */
}

static void Codec_VolumeCtrl(uint8_t volume) {
	if (volume > 0xE6) { /* Set the Master volume */
		Codec_WriteRegister(0x20, volume - 0xE7);
		Codec_WriteRegister(0x21, volume - 0xE7);
	} else { /* Set the Master volume */
		Codec_WriteRegister(0x20, volume + 0x19);
		Codec_WriteRegister(0x21, volume + 0x19);
	}
}

static void Codec_AudioInterface_Init(uint32_t audioFreq) {
	__SPI3_CLK_ENABLE();

	AudioOut.hi2s.Instance = CODEC_I2S;
	HAL_I2S_DeInit(&AudioOut.hi2s);
	AudioOut.hi2s.Init.AudioFreq = audioFreq;
	AudioOut.hi2s.Init.Standard = I2S_STANDARD_PHILIPS;
	AudioOut.hi2s.Init.DataFormat = I2S_DATAFORMAT_16B;
	AudioOut.hi2s.Init.CPOL = I2S_CPOL_LOW;
	AudioOut.hi2s.Init.Mode = I2S_MODE_MASTER_TX;
	AudioOut.hi2s.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
	AudioOut.hi2s.hdmatx = &AudioOut.hdma;
	HAL_I2S_Init(&AudioOut.hi2s);
}

static void Audio_MAL_Init() {
	__DMA1_CLK_ENABLE();

	AudioOut.hdma.Instance = AUDIO_I2S_DMA_STREAM;
	HAL_DMA_DeInit(&AudioOut.hdma);

	AudioOut.hdma.Init.Channel = AUDIO_I2S_DMA_CHANNEL;
	AudioOut.hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
	AudioOut.hdma.Init.PeriphInc = DMA_PINC_DISABLE;
	AudioOut.hdma.Init.MemInc = DMA_MINC_ENABLE;
	AudioOut.hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	AudioOut.hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	AudioOut.hdma.Init.Mode = DMA_NORMAL;
	AudioOut.hdma.Init.Priority = DMA_PRIORITY_HIGH;
	AudioOut.hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	AudioOut.hdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
	AudioOut.hdma.Init.MemBurst = DMA_MBURST_SINGLE;
	AudioOut.hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
	HAL_DMA_Init(&AudioOut.hdma);

	AUDIO_I2S_DMA_STREAM->PAR = (uint32_t) & (CODEC_I2S->DR); /* Configure DMA Channel source address */

	HAL_NVIC_SetPriority(AUDIO_I2S_DMA_IRQ, 2, 0);
	HAL_NVIC_EnableIRQ(AUDIO_I2S_DMA_IRQ);
}

static void Codec_Mute(TCodecAudioMute cmd) {
	switch (cmd) {
		case TCodecAudioMute::AUDIO_MUTE_ON:
			Codec_WriteRegister(0x04, 0xFF);
			break;
		case TCodecAudioMute::AUDIO_MUTE_OFF:
			Codec_WriteRegister(0x04, AudioOut.Codec.OutputDev);
			break;
	}
}

static void Codec_Stop() {
	Codec_Mute(AUDIO_MUTE_ON);		 /* Mute the output first */
	Codec_WriteRegister(0x02, 0x9F); /* Power down the DAC components */
}

static void DmaTransmit(uint16_t *pData, uint16_t size) {
	DMA_Base_Registers *regs = txDmaRegisters;

	AUDIO_I2S_DMA_STREAM->CR &= (uint32_t)(~DMA_SxCR_DBM);			   /* Clear DBM bit */
	AUDIO_I2S_DMA_STREAM->CR &= ~DMA_SxCR_EN;						   /* Disable the peripheral */
	AUDIO_I2S_DMA_STREAM->CR &= ~(DMA_IT_TC | DMA_IT_TE | DMA_IT_DME); /* Disable all the transfer interrupts */
	AUDIO_I2S_DMA_STREAM->FCR &= ~(DMA_IT_FE);

	AUDIO_I2S_DMA_STREAM->M0AR = (uint32_t)pData; /* Configure DMA Stream source address */
	AUDIO_I2S_DMA_STREAM->NDTR = size;			  /* Configure DMA Stream data length */

	regs->IFCR = 0x3FU << txDmaStreamIndex; /* Clear all interrupt flags at correct offset within the register */

	AUDIO_I2S_DMA_STREAM->CR |= DMA_IT_TC | DMA_IT_TE | DMA_IT_DME; /* Enable Common interrupts*/
	AUDIO_I2S_DMA_STREAM->CR |= DMA_SxCR_EN;						/* Enable the Peripheral */

	__HAL_I2S_ENABLE(&AudioOut.hi2s);		  /* Enable I2S peripheral */
	SET_BIT(CODEC_I2S->CR2, SPI_CR2_TXDMAEN); /* Enable Tx DMA Request */
}

extern "C" void DMA1_Stream7_IRQHandler(void) {
	DMA_Base_Registers *regs = txDmaRegisters;
	uint32_t tmpisr = regs->ISR;
	if ((tmpisr & (DMA_FLAG_TEIF0_4 << txDmaStreamIndex)) != RESET) {		 /* Transfer Error Interrupt management ***************************************/
		AUDIO_I2S_DMA_STREAM->CR &= ~(DMA_IT_TE);							 /* Disable the transfer error interrupt */
		regs->IFCR = DMA_FLAG_TEIF0_4 << txDmaStreamIndex;					 /* Clear the transfer error flag */
	} else if ((tmpisr & (DMA_FLAG_TEIF0_4 << txDmaStreamIndex)) != RESET) { /* Direct Mode Error Interrupt management ***********************************/
		regs->IFCR = DMA_FLAG_DMEIF0_4 << txDmaStreamIndex;					 /* Clear the direct mode error flag */
	} else if ((tmpisr & (DMA_FLAG_TCIF0_4 << txDmaStreamIndex)) != RESET) { /* Transfer Complete Interrupt management ***********************************/
		AUDIO_I2S_DMA_STREAM->CR &= ~(DMA_IT_TC | DMA_IT_TE | DMA_IT_DME);   /* Disable all the transfer interrupts */
		AUDIO_I2S_DMA_STREAM->FCR &= ~(DMA_IT_FE);
		regs->IFCR = DMA_FLAG_TCIF0_4 << txDmaStreamIndex; /* Clear the transfer complete flag */

		if (AudioOut.Codec.CircularPlay && AudioOut.Codec.RemainingSize <= 0) {
			AudioOut.Codec.CurrentPos -= AudioOut.Codec.TotalSize;
			AudioOut.Codec.RemainingSize = AudioOut.Codec.TotalSize;
		}
		if (AudioOut.Codec.RemainingSize > 0) {
			AudioOut.Codec.CurrentPos += DMA_MAX(AudioOut.Codec.RemainingSize);
			DmaTransmit(AudioOut.Codec.CurrentPos, DMA_MAX(AudioOut.Codec.RemainingSize));
			AudioOut.Codec.RemainingSize -= DMA_MAX(AudioOut.Codec.RemainingSize);
		}
	}
}
