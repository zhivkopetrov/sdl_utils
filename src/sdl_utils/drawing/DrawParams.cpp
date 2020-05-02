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
  center(&centerPos),
  opacity(FULL_OPACITY),
  widgetType(WidgetType::UNKNOWN),
  hasCrop(false),
  hasScaling(false) {

}

void DrawParams::reset() {
    rsrcId = 0;
    pos.x = 0;
    pos.y = 0;
    centerPos.x = 0;
    centerPos.y = 0;
    center = &centerPos;
    scaledWidth = 0;
    scaledHeight = 0;
    angle = 0.0;
    opacity = FULL_OPACITY;
    hasCrop = false;
    hasScaling = false;
}

