// Corresponding header
#include "sdl_utils/drawing/MonitorWindow.h"

// C system headers

// C++ system headers
#include <string>

// Other libraries headers
#include <SDL_video.h>
#include <SDL_image.h>

// Own components headers
#include "sdl_utils/drawing/defines/RendererDefines.h"

#if !USE_SOFTWARE_RENDERER
#include "sdl_utils/drawing/Texture.h"
#endif /* !USE_SOFTWARE_RENDERER */

#include "utils/Log.h"

// Window modes:
// SDL_WINDOW_SHOWN - for windowed version
// SDL_WINDOW_FULLSCREEN_DESKTOP - for fullscreen
// SDL_WINDOW_OPENGL for working with OPEN_GL windows and context

MonitorWindow::MonitorWindow(const int32_t windowWidth,
                             const int32_t windowHeight, const Point& windowPos)
    : _window(nullptr),
      _windowRect(windowPos.x, windowPos.y, windowWidth, windowHeight) {}

MonitorWindow::MonitorWindow(const Rectangle& rect)
    : _window(nullptr), _windowRect(rect) {}

MonitorWindow::~MonitorWindow() { deinit(); }

int32_t MonitorWindow::init(const WindowDisplayMode displayMode,
                            const WindowBorderMode borderMode) {
  int32_t initWindowX = 0;
  int32_t initWindowY = 0;

  const Point POS(_windowRect.x, _windowRect.y);

  if (Point::UNDEFINED == POS) {
    initWindowX = SDL_WINDOWPOS_UNDEFINED;
    initWindowY = SDL_WINDOWPOS_UNDEFINED;
  } else {
    initWindowX = _windowRect.x;
    initWindowY = _windowRect.y;
  }

  // Create window
  _window = SDL_CreateWindow("GameWindow", initWindowX, initWindowY,
                             _windowRect.w, _windowRect.h,
                             getValue(displayMode, borderMode));

  if (nullptr == _window) {
    LOGERR("Window could not be created! SDL Error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  // obtain real window coordinates after creation
  SDL_GetWindowPosition(_window, &_windowRect.x, &_windowRect.y);

#if !USE_SOFTWARE_RENDERER
  Texture::setMonitorRect(_windowRect);
#endif /* USE_SOFTWARE_RENDERER */

  return EXIT_SUCCESS;
}

void MonitorWindow::deinit() {
  if (_window)  // sanity check
  {
    SDL_DestroyWindow(_window);
    _window = nullptr;
  }
}

int32_t MonitorWindow::loadWindowIcon(const char *iconPath) {
  SDL_Surface* windowIcon = IMG_Load(iconPath);
  if (nullptr == windowIcon) {
    LOGERR("Unable to create window Image from file! SDL Error: %s",
           SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_SetWindowIcon(_window, windowIcon);
  SDL_FreeSurface(windowIcon);
  windowIcon = nullptr;
  return EXIT_SUCCESS;
}

