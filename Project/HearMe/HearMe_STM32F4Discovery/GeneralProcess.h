#pragma once

typedef struct {
	uint32_t timerHeartbeat;
	
} TGeneralProcessTimers;

typedef struct {
	TaskHandle_t TaskHandle;
	TGeneralProcessTimers timers;

} TGeneralProcess, *PTGeneralProcess;

extern TGeneralProcess GeneralProcess;

void InitGeneralProcess();
void TaskGeneralProcess(void *arg);
