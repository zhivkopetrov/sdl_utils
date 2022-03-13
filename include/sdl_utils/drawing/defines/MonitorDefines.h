#ifndef SDL_UTILS_MONITORDEFINES_H_
#define SDL_UTILS_MONITORDEFINES_H_

// System headers
#include <cstdint>

// Other libraries headers

// Own components headers

// Forward declarations

enum class WindowDisplayMode {
  UNKNOWN, FULL_SCREEN, WINDOWED
};

enum class WindowBorderMode {
  UNKNOWN, WITH_BORDER, BORDERLESS
};

int32_t getValue(const WindowDisplayMode displayMode,
                 const WindowBorderMode borderMode);

#endif /* SDL_UTILS_MONITORDEFINES_H_ */
