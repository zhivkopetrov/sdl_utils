#define USE_WINDOW_ICON 1

// Corresponding header
#include "sdl_utils/drawing/MonitorWindow.h"

// C system headers

// C++ system headers
#include <string>

// Other libraries headers
#include <SDL_video.h>

#if USE_WINDOW_ICON
#include <SDL_image.h>
#endif  // USE_WINDOW_ICON

// Own components headers
#include "sdl_utils/drawing/RendererDefines.h"

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

int32_t MonitorWindow::init(const std::string &projectFolderName,
                            const int32_t displayMode) {
  int32_t err = EXIT_SUCCESS;

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
                             _windowRect.w, _windowRect.h, displayMode);

  if (nullptr == _window) {
    LOGERR("Window could not be created! SDL Error: %s", SDL_GetError());

    err = EXIT_FAILURE;
  } else {
    // obtain real window coordinates after creation
    SDL_GetWindowPosition(_window, &_windowRect.x, &_windowRect.y);
  }

#if !USE_SOFTWARE_RENDERER
  Texture::setMonitorRect(_windowRect);
#endif /* USE_SOFTWARE_RENDERER */

#if USE_WINDOW_ICON
  if (EXIT_SUCCESS == err) {
    const std::string absoluteFilePath = __FILE__;
    const std::string PROJECT_FOLDER = projectFolderName + "/";

    // use rfind, because we are closer to the end
    const uint64_t currDirPos = absoluteFilePath.rfind(PROJECT_FOLDER);

    std::string projectFilePath = "";

    if (std::string::npos == currDirPos) {
      LOGERR("Error, project folder not found");

      err = EXIT_FAILURE;
    } else {
      projectFilePath =
          absoluteFilePath.substr(0, currDirPos + PROJECT_FOLDER.size());

      std::string ICON_FILE_PATH = projectFilePath;
      ICON_FILE_PATH.append(
          "/commonresources/p/loadingscreen/CHANGE_ME.jpg");

      SDL_Surface* windowIcon = IMG_Load(ICON_FILE_PATH.c_str());

      if (nullptr == windowIcon) {
        LOGERR("Unable to create window Image from file! SDL Error: %s",
               SDL_GetError());

        err = EXIT_FAILURE;
      } else {
        SDL_SetWindowIcon(_window, windowIcon);

        SDL_FreeSurface(windowIcon);
        windowIcon = nullptr;
      }
    }
  }
#endif  // USE_WINDOW_ICON

  return err;
}

void MonitorWindow::deinit() {
  if (_window)  // sanity check
  {
    SDL_DestroyWindow(_window);
    _window = nullptr;
  }
}
