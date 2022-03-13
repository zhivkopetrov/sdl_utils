//Corresponding header
#include "sdl_utils/drawing/defines/MonitorDefines.h"

//System headers

//Other libraries headers
#include <SDL_video.h>
#include "utils/data_type/EnumClassUtils.h"
#include "utils/Log.h"

//Own components headers

// Window modes:
// SDL_WINDOW_SHOWN - for windowed version
// SDL_WINDOW_FULLSCREEN_DESKTOP - for fullscreen
// SDL_WINDOW_OPENGL for working with OPEN_GL windows and context

int32_t getValue(const WindowDisplayMode displayMode,
		const WindowBorderMode borderMode) {
	switch (borderMode) {
	case WindowBorderMode::WITH_BORDER:
		[[fallthrough]];
	case WindowBorderMode::BORDERLESS:
		break;
	default:
		LOGERR("Received unknown WindowBorderMode code: %d",
				getEnumValue(borderMode));
		return 0;
	}

	switch (displayMode) {
	case WindowDisplayMode::FULL_SCREEN:
		return WindowBorderMode::BORDERLESS == borderMode ?
				(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS) :
				SDL_WINDOW_FULLSCREEN;
	case WindowDisplayMode::WINDOWED:
		return WindowBorderMode::BORDERLESS == borderMode ?
				(SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS) : SDL_WINDOW_SHOWN;
	default:
		LOGERR("Received unknown DisplayMode code: %d",
				getEnumValue(displayMode));
	}
	return 0;
}
