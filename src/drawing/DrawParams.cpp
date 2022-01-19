// Corresponding header
#include "sdl_utils/drawing/DrawParams.h"

// C system headers

// C++ system headers

// Other libraries headers

// Own components headers

DrawParams::DrawParams() :
  rsrcId(0),
  scaledWidth(0),
  scaledHeight(0),
  angle(0.0),
  opacity(FULL_OPACITY),
  widgetType(WidgetType::UNKNOWN),
  widgetFlipType(WidgetFlipType::NONE),
  hasCrop(false),
  hasScaling(false) {

}

void DrawParams::reset() {
    rsrcId = 0;
    pos = Point::ZERO;
    rotCenter = Point::ZERO;
    scaledWidth = 0;
    scaledHeight = 0;
    angle = 0.0;
    opacity = FULL_OPACITY;
    hasCrop = false;
    hasScaling = false;
}

