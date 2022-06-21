#ifndef SDL_UTILS_DRAWCONSTANTS_H_
#define SDL_UTILS_DRAWCONSTANTS_H_

// System headers
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

enum class WidgetFlipType : uint8_t {
  NONE = 0,
  VERTICAL = 1,
  HORIZONTAL = 2,
  VERTICAL_AND_HORIZONTAL = 3
};

enum class BlendMode : uint8_t {
  NONE = 0,
  BLEND = 1,
  ADD = 2
};

enum class RotationCenterType : uint8_t {
  TOP_LEFT,
  ORIG_CENTER,
  SCALED_CENTER
};

enum class ScreenshotContainer : uint8_t {
  PNG, JPG
};

constexpr int32_t ZERO_OPACITY = 0;
constexpr int32_t FULL_OPACITY = 255;

constexpr double ZERO_ANGLE = 0.0;
constexpr double FULL_ROTATION_ANGLE = 360.0;

constexpr double MIN_SCALE_FACTOR = 0.0;
constexpr double MAX_SCALE_FACTOR = 1.0;

#endif /* SDL_UTILS_DRAWCONSTANTS_H_ */
