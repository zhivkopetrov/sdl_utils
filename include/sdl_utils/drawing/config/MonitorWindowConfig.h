#ifndef SDL_UTILS_DRAWING_CONFIG_MONITORWINDOWCONFIG_H_
#define SDL_UTILS_DRAWING_CONFIG_MONITORWINDOWCONFIG_H_

// System headers
#include <cstdint>
#include <string>

// Other libraries headers
#include "utils/drawing/Point.h"

// Own components headers
#include "sdl_utils/drawing/defines/MonitorDefines.h"

// Forward declarations

/*
 *  @param std::string&      - name of the new window
 *  @param int32_t           - relative path to window icon [optional]
 *  @param int32_t           - the width of the new window
 *  @param int32_t           - the height of the new window
 *  @param Point             - the position of the new window
 *                             NOTE: the provided position has absolute
 *                             monitor coordinates.
 *  @param WindowDisplayMode - full screen or windowed
 *  @param WindowBorderMode  - borderless or not
 *
 *   If not coordinates for the windows were provided (Point::UNDEFINED)
 *   the new windows is created as CENTERED to the hardware monitor.
 */
struct MonitorWindowConfig {
  std::string name;
  std::string iconPath; //optional
  Point pos;
  int32_t width = 0;
  int32_t height = 0;
  WindowDisplayMode displayMode = WindowDisplayMode::UNKNOWN;
  WindowBorderMode borderMode = WindowBorderMode::UNKNOWN;
};

#endif /* SDL_UTILS_DRAWING_CONFIG_MONITORWINDOWCONFIG_H_ */
