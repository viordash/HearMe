
#include "Board.h"
#include "Button.h"
#include "TimeHelper.h"

typedef enum { //
	bd_Idle,
	bd_Released,
	bd_Pressed

} TButtonDetection;

typedef struct {
	uint32_t stateChangesTimer;
	uint32_t pressedTimer;
	TButtonDetection detection;
	bool pressed;
} TButton, *PTButton;

TButton Button;

void InitButton() {
	memset(&Button, 0, sizeof(Button));
	Button.detection = TButtonDetection::bd_Idle;
}

TButtonState ButtonPeriodic() {
	bool pressed = ReadPortPin(BUTTON_PORT, BUTTON_PIN);
	if (pressed != Button.pressed) {
		Button.pressed = pressed;

		Button.stateChangesTimer = SysTickCount;
		if (pressed) {
			Button.detection = TButtonDetection::bd_Pressed;
		} else {
			Button.detection = TButtonDetection::bd_Released;
		}
	}

	switch (Button.detection) {
		case TButtonDetection::bd_Pressed:
		case TButtonDetection::bd_Released: {
			bool debounceFilter = PeriodInRange(Button.stateChangesTimer, 20);
			if (!debounceFilter) {
				return pressed ? TButtonState::Pressed : TButtonState::Released;
			}
			break;
		}
		default:
			return pressed ? TButtonState::Pressed : TButtonState::Released;
	}

	switch (Button.detection) {
		case TButtonDetection::bd_Pressed:
			Button.detection = TButtonDetection::bd_Idle;
			Button.pressedTimer = SysTickCount;
			return TButtonState::OnKeyDown;

		case TButtonDetection::bd_Released: {
			Button.detection = TButtonDetection::bd_Idle;
			uint32_t duration = GetDuration(Button.pressedTimer);
			if (duration >= 10000) {
				return TButtonState::LongPress;
			}
			if (duration >= 300) {
				return TButtonState::MidPress;
			}
			return TButtonState::ShortPress;
		}

		default:
			return pressed ? TButtonState::Pressed : TButtonState::Released;
	}
}
