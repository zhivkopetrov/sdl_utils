#ifndef SDL_UTILS_DRAWUTILITYDEFINES_H_
#define SDL_UTILS_DRAWUTILITYDEFINES_H_

//System headers
#include <cstdint>
#include <functional>

//Other libraries headers

//Own components headers
#include "sdl_utils/drawing/defines/DrawConstants.h"

using TakeScreenshotCb = std::function<void(const char *file,
    const ScreenshotContainer container, const int32_t quality)>;

#endif /* SDL_UTILS_DRAWUTILITYDEFINES_H_ */
