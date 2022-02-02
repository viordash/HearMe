#pragma once

typedef enum { Off, Br1, Br10, Br50, Br75, On, FastFlash, Flash, Pulse, FastOff, FastOn, FastFastFlash } TLedMode;

#define FLASH_PERIOD_MS 700
#define FAST_FLASH_PERIOD_MS 200
#define FAST_FAST_FLASH_PERIOD_MS 50
#define PULSE_DURATION_MS 10

void InitLeds();
void StartLeds();

void ChangeBlueLed(TLedMode newMode);
void ChangeGreenLed(TLedMode newMode);
void ChangeRedLed(TLedMode newMode);
void ChangeOrangeLed(TLedMode newMode);

TLedMode GetBlueLedStatus();
TLedMode GetGreenLedStatus();
TLedMode GetRedLedStatus();
TLedMode GetOrangeLedStatus();
