
#include "Board.h"
#include "TimeHelper.h"
#include "time.h"

uint32_t GetDuration(uint32_t watchedTime) {
	uint32_t duration;
	if (SysTickCount > watchedTime) {
		duration = SysTickCount - watchedTime;
	} else {
		duration = (4294967295U - (watchedTime - SysTickCount)) + 1;
	}
	return duration;
}

bool PeriodInRange(uint32_t watchedTime, uint32_t minTime, uint32_t maxTime) {
	uint32_t duration = GetDuration(watchedTime);
	return duration >= minTime && duration <= maxTime;
}

bool PeriodInRange(uint32_t watchedTime, uint32_t minTime) {
	uint32_t duration = GetDuration(watchedTime);
	return duration >= minTime;
}
