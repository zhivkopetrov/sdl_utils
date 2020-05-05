#ifndef SDL_UTILS_DRAWCONSTANTS_H_
#define SDL_UTILS_DRAWCONSTANTS_H_

// C system headers

// C++ system headers
#include <cstdint>

// Other libraries headers

// Own components headers

// Forward declarations

enum class WidgetType : uint8_t {
  IMAGE,
  TEXT,
  SPRITE_BUFFER,

  UNKNOWN = 255
};

constexpr inline int32_t ZERO_OPACITY = 0;
constexpr inline int32_t FULL_OPACITY = 255;

constexpr inline double ZERO_ANGLE = 0.0;
constexpr inline double FULL_ROTATION_ANGLE = 360.0;

constexpr inline double MIN_SCALE_FACTOR = 0.0;
constexpr inline double MAX_SCALE_FACTOR = 1.0;

#endif /* SDL_UTILS_DRAWCONSTANTS_H_ */
