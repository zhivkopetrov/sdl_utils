//Corresponding header
#include "sdl_utils/drawing/defines/MonitorDefines.h"

//C system headers

//C++ system headers

//Other libraries headers
#include <SDL_video.h>

//Own components headers
#include "utils/data_type/EnumClassUtils.hpp"
#include "utils/Log.h"

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
