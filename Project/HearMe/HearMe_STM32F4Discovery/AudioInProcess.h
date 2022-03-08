#pragma once

typedef struct {
	uint32_t timerHeartbeat;
	
} TAudioInProcessTimers;

typedef struct {
	TaskHandle_t TaskHandle;
	TAudioInProcessTimers timers;

} TAudioInProcess, *PTAudioInProcess;

extern TAudioInProcess AudioInProcess;

void InitAudioInProcess();
void TaskAudioInProcess(void *arg);
