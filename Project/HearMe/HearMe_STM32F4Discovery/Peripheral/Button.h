#pragma once

typedef enum { //
	Pressed,
	Released,
	OnKeyDown,
	ShortPress,
	MidPress,
	LongPress
} TButtonState;

void InitButton();
TButtonState ButtonPeriodic();
