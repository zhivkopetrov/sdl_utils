#ifndef SDL_UTILS_DRAWPARAMS_H_
#define SDL_UTILS_DRAWPARAMS_H_

// C system headers

// C++ system headers
#include <cstdint>

// Other libraries headers

// Own components headers
#include "utils/drawing/Point.h"
#include "utils/drawing/Rectangle.h"

// Forward declarations

enum class WidgetType : uint8_t {
  IMAGE,
  TEXT,
  SPRITE_BUFFER,

  UNKNOWN = 255
};

constexpr inline int32_t ZERO_OPACITY = 0;
constexpr inline int32_t FULL_OPACITY = 255;

struct DrawParams {
  DrawParams();

  void reset();

  union {
      // Hold the resource hash
      uint64_t rsrcId;

      //unique textId
      int32_t textId;

      //unique sprite buffer id
      int32_t spriteBufferId;
  };

  // Top left position of texture
  Point pos;

  // Position for center of rotation
  Point centerPos;

  // Scaled Widget single frame dimensions
  int32_t scaledWidth;
  int32_t scaledHeight;

  // Angle of rotation
  double angle;

  // Center for rotation
  Point* center;

  // Rectangle for bounding the currently set frame
  Rectangle frameRect;

  /** Holds the frame crop rectangle to be drawn
   * NOTE: frameCropRect.x and frameCropRect.y represent
   * X and Y position coordinates for the renderQuad rectangle
   * */
  Rectangle frameCropRect;

  // Holds Widget opacity (where 0 is full transparent and 255 is full opacity)
  int32_t opacity;

  // Holds the specific widget type (IMAGE, TEXT, SPRITE_BUFFER, etc...)
  WidgetType widgetType;

  // A flag to determine whether crop is activated for current Widget
  bool hasCrop;

  // A flag to determine whether scaling is enabled for the current Widget
  bool hasScaling;
};

#endif /* SDL_UTILS_DRAWPARAMS_H_ */
