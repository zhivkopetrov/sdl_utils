#ifndef SDL_UTILS_MONITORWINDOW_H_
#define SDL_UTILS_MONITORWINDOW_H_

// C system headers

// C++ system headers
#include <cstdint>

// Other libraries headers

// Own components headers
#include "utils/drawing/Point.h"
#include "utils/drawing/Rectangle.h"

// Forward declarations
struct SDL_Window;

class MonitorWindow {
 public:
  /** @brief used to create window rectangle from provided
   *                                              individual dimensions
   *
   *  @param const int32_t - the width of the new window
   *  @param const int32_t - the height of the new window
   *  @param const Point   - the position of the new window
   *                         NOTE: the provided position has absolute
   *                         monitor coordinates.
   * */
  explicit MonitorWindow(const int32_t windowWidth, const int32_t windowHeight,
                         const Point& windowPos = Point::UNDEFINED);

  /** @brief used to create window rectangle from provided
   *                                               Rectangle dimensions
   *
   *  @param const Rectangle - the window Rectangle dimensions
   *                         NOTE: the provided position has absolute
   *                         monitor coordinates.
   * */
  explicit MonitorWindow(const Rectangle& rect);

  // forbid the copy and move constructors
  MonitorWindow(const MonitorWindow& other) = delete;
  MonitorWindow(MonitorWindow&& other) = delete;

  // forbid the copy and move assignment operators
  MonitorWindow& operator=(const MonitorWindow& other) = delete;
  MonitorWindow& operator=(MonitorWindow&& other) = delete;

  ~MonitorWindow();

  /** @brief used to create window from previously set _windowRect
   *
   *   If not coordinates for the windows were provided (Point::UNDEFINED)
   *   the new windows is created as CENTERED to the hardware monitor.
   *
   *  @param const int32_t - display mode for the window
   *
   *  @returns int32_t     - error code
   * */
  int32_t init(const int32_t displayMode);

  /** @brief used to deinitialize the created window
   * */
  void deinit();

  /** @brief used to get access to the the actual created window
   * */
  inline SDL_Window* getWindow() const { return _window; }

  /** @brief loads window icon (on the dash bar) for the application window
   *
   *  @param const char * - path to image
   *
   *  @return int32_t     - error code
   * */
  int32_t loadWindowIcon(const char *iconPath);

 private:
  // The actual window
  SDL_Window* _window;

  // Holds window position and dimensions
  Rectangle _windowRect;
};

#endif /* SDL_UTILS_MONITORWINDOW_H_ */
