#ifndef SDL_UTILS_MONITORWINDOW_H_
#define SDL_UTILS_MONITORWINDOW_H_

// C system headers

// C++ system headers
#include <cstdint>

// Other libraries headers
#include "utils/class/NonCopyable.h"
#include "utils/class/NonMoveable.h"
#include "utils/drawing/Rectangle.h"

// Own components headers
#include "sdl_utils/drawing/config/MonitorWindowConfig.h"

// Forward declarations
struct SDL_Window;

class MonitorWindow : public NonCopyable, public NonMoveable {
 public:
  MonitorWindow() = default;
  ~MonitorWindow();

  int32_t init(const MonitorWindowConfig& cfg);

  /** @brief used to deinitialize the created window
   * */
  void deinit();

  /** @brief used to get access to the the actual created window
   * */
  SDL_Window* getNativeWindow() const { return _window; }

  /** @brief loads window icon (on the dash bar) for the application window
   *
   *  @param const std::string& - path to image
   *
   *  @return int32_t           - error code
   * */
  int32_t loadWindowIcon(const std::string& iconPath);

 private:
  // The actual window
  SDL_Window* _window = nullptr;

  // Holds window position and dimensions
  Rectangle _windowRect = Rectangle::UNDEFINED;
};

#endif /* SDL_UTILS_MONITORWINDOW_H_ */
