

#include "Board.h"
#include "Leds.h"

typedef struct {
	TLedMode Status;
	uint16_t Brightness;
	uint16_t Timer;
	uint8_t Ramp;
} TLed, *PTLed;

typedef struct {
	TLed Blue;
	TLed Green;
	TLed Red;
	TLed Orange;
	uint16_t ReloadValueForTimer1S;
} TLeds, *PTLeds;

TLeds ButtonsLeds;
#define TIM_RELOAD_VALUE_FOR_1S (HAL_RCC_GetPCLK1Freq() * 2 / (TIM4->PSC + 1) / (TIM4->ARR + 1))

void InitLeds() {
	TIM_HandleTypeDef htim4 = {0};
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 15;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 199;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim4) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK) {
		Error_Handler();
	}
	ButtonsLeds.Blue.Status = TLedMode::Off;
	ButtonsLeds.Blue.Brightness = 0;
	ButtonsLeds.Blue.Ramp = 0;
	ButtonsLeds.Green.Status = TLedMode::Off;
	ButtonsLeds.Green.Brightness = 0;
	ButtonsLeds.Green.Ramp = 0;
	ButtonsLeds.Red.Status = TLedMode::Off;
	ButtonsLeds.Red.Brightness = 0;
	ButtonsLeds.Red.Ramp = 0;
	ButtonsLeds.Orange.Status = TLedMode::Off;
	ButtonsLeds.Orange.Brightness = 0;
	ButtonsLeds.Orange.Ramp = 0;
	ButtonsLeds.ReloadValueForTimer1S = TIM_RELOAD_VALUE_FOR_1S;
}

void StartLeds() {
	TIM_HandleTypeDef htim4;
	htim4.Instance = TIM4;
	if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4) != HAL_OK) {
		Error_Handler();
	}
	__HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);
	HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

#define SetButtonLedBrightness(pLed, brightnessPercent)                                                                                                        \
	{ (pLed)->Brightness = ((TIM4->ARR + 1) * brightnessPercent) / 100; }

#define IsButtonLedOn(pLed) ((pLed)->Brightness > 0)

void ChangeLedStatus(PTLed pLed, TLedMode newMode) {
	HAL_NVIC_DisableIRQ(TIM4_IRQn);
	if (pLed->Status == newMode && newMode != TLedMode::Pulse) {
		HAL_NVIC_EnableIRQ(TIM4_IRQn);
		return;
	}
	switch (newMode) {
		case Br1:
			SetButtonLedBrightness(pLed, 1);
			break;
		case Br10:
			SetButtonLedBrightness(pLed, 10);
			break;
		case Br50:
			SetButtonLedBrightness(pLed, 50);
			break;
		case Br75:
			SetButtonLedBrightness(pLed, 75);
			break;
		case On:
		case FastOn:
			SetButtonLedBrightness(pLed, 100);
			break;

		case Flash:
		case FastFlash:
		case FastFastFlash:
			pLed->Timer = 0;
			pLed->Ramp = 0;
			break;

		case Pulse:
			pLed->Timer = ButtonsLeds.ReloadValueForTimer1S / (1000 / PULSE_DURATION_MS);
			pLed->Ramp = 0;
			SetButtonLedBrightness(pLed, 100);
			break;

		default:
			SetButtonLedBrightness(pLed, 0);
			break;
	}
	pLed->Status = newMode;
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

void ChangeBlueLed(TLedMode newMode) {
	ChangeLedStatus(&ButtonsLeds.Blue, newMode);
}

void ChangeGreenLed(TLedMode newMode) {
	ChangeLedStatus(&ButtonsLeds.Green, newMode);
}

void ChangeRedLed(TLedMode newMode) {
	ChangeLedStatus(&ButtonsLeds.Red, newMode);
}

void ChangeOrangeLed(TLedMode newMode) {
	ChangeLedStatus(&ButtonsLeds.Orange, newMode);
}

TLedMode GetBlueLedStatus() {
	return ButtonsLeds.Blue.Status;
}

TLedMode GetGreenLedStatus() {
	return ButtonsLeds.Green.Status;
}

TLedMode GetRedLedStatus() {
	return ButtonsLeds.Red.Status;
}

TLedMode GetOrangeLedStatus() {
	return ButtonsLeds.Orange.Status;
}

void FlashMode(PTLed pLed, uint32_t *pCCR, uint8_t ramp, uint16_t timer) {
	if (pLed->Timer == 0) {
		if (*pCCR != pLed->Brightness) {
			if (pLed->Ramp++ >= ramp) {
				if (*pCCR > pLed->Brightness) {
					(*pCCR)--;
				} else if (*pCCR < pLed->Brightness) {
					(*pCCR)++;
				}
				pLed->Ramp = 0;
			}
			return;
		}

		if (IsButtonLedOn(pLed)) {
			SetButtonLedBrightness(pLed, 0);
		} else {
			SetButtonLedBrightness(pLed, 100);
		}
		pLed->Timer = timer;
	} else {
		pLed->Timer--;
	}
}

void LedsPeriodic(PTLed pLed, uint32_t *pCCR) {
	switch (pLed->Status) {
		case FastFastFlash:
			FlashMode(pLed, pCCR, 5, (ButtonsLeds.ReloadValueForTimer1S / 1000) * FAST_FAST_FLASH_PERIOD_MS);
			break;

		case FastFlash:
			FlashMode(pLed, pCCR, 5, (ButtonsLeds.ReloadValueForTimer1S / 1000) * FAST_FLASH_PERIOD_MS);
			break;

		case Flash:
			FlashMode(pLed, pCCR, 20, (ButtonsLeds.ReloadValueForTimer1S / 1000) * FLASH_PERIOD_MS);
			break;

		case Pulse:
			if (*pCCR != pLed->Brightness) {
				if (pLed->Ramp++ >= 2) {
					if (*pCCR > pLed->Brightness) {
						(*pCCR)--;
					} else if (*pCCR < pLed->Brightness) {
						(*pCCR)++;
					}
					pLed->Ramp = 0;
				}
				break;
			}

			if (pLed->Timer == 0) {
				if (IsButtonLedOn(pLed)) {
					SetButtonLedBrightness(pLed, 0);
				} else {
					ChangeLedStatus(pLed, TLedMode::Off);
				}
			} else {
				pLed->Timer--;
			}
			break;

		case Br1:
		case Br10:
		case Br50:
		case Br75:
			if (*pCCR != pLed->Brightness) {
				if (pLed->Ramp++ >= 20) {
					if (*pCCR > pLed->Brightness) {
						(*pCCR)--;
					} else if (*pCCR < pLed->Brightness) {
						(*pCCR)++;
					}
					pLed->Ramp = 0;
				}
			}

		case On:
		case Off:
			if (*pCCR != pLed->Brightness) {
				if (pLed->Ramp++ >= 10) {
					if (*pCCR > pLed->Brightness) {
						(*pCCR)--;
					} else if (*pCCR < pLed->Brightness) {
						(*pCCR)++;
					}
					pLed->Ramp = 0;
				}
			}
			break;

		case FastOn:
		case FastOff:
			*pCCR = pLed->Brightness;
			break;

		default:
			break;
	}
}

#ifdef __cplusplus
extern "C" {
#endif

void TIM4_IRQHandler(void) {
	TIM_HandleTypeDef htim4;
	htim4.Instance = TIM4;
	TIM_HandleTypeDef *htim = &htim4;

	if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_UPDATE) != RESET) {
		if (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_UPDATE) != RESET) {
			__HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);

			LedsPeriodic(&ButtonsLeds.Green, (uint32_t *)&(TIM4->CCR1));
			LedsPeriodic(&ButtonsLeds.Orange, (uint32_t *)&(TIM4->CCR2));
			LedsPeriodic(&ButtonsLeds.Red, (uint32_t *)&(TIM4->CCR3));
			LedsPeriodic(&ButtonsLeds.Blue, (uint32_t *)&(TIM4->CCR4));
		}
	}
}

#ifdef __cplusplus
}
#endif