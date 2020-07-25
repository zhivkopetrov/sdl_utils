#ifndef SDL_UTILS_MONITORDEFINES_H_
#define SDL_UTILS_MONITORDEFINES_H_

// C system headers

// C++ system headers
#include <cstdint>

// Other libraries headers

// Own components headers

// Forward declarations

enum class WindowDisplayMode {
  UNKNOWN,
  FULL_SCREEN,
  WINDOWED
};

enum class WindowBorderMode {
  UNKNOWN,
  WITH_BORDER,
  BORDERLESS
};

int getValue(const WindowDisplayMode displayMode,
             const WindowBorderMode borderMode);

#endif /* SDL_UTILS_MONITORDEFINES_H_ */