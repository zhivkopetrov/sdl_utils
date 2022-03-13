// Corresponding header
#include "sdl_utils/drawing/MonitorWindow.h"

// System headers
#include <string>

// Other libraries headers
#include <SDL_video.h>
#include <SDL_image.h>
#include "utils/Log.h"

// Own components headers
#include "sdl_utils/drawing/defines/RendererDefines.h"
#include "sdl_utils/drawing/Texture.h"
#include "sdl_utils/drawing/LoadingScreen.h"

MonitorWindow::~MonitorWindow() noexcept {
  deinit();
}

ErrorCode MonitorWindow::init(const MonitorWindowConfig& cfg) {
  _windowRect = Rectangle(cfg.pos, cfg.width, cfg.height);

  int32_t initWindowX = 0;
  int32_t initWindowY = 0;

  const Point POS(_windowRect.x, _windowRect.y);

  if (Points::UNDEFINED == POS) {
    initWindowX = SDL_WINDOWPOS_UNDEFINED;
    initWindowY = SDL_WINDOWPOS_UNDEFINED;
  } else {
    initWindowX = _windowRect.x;
    initWindowY = _windowRect.y;
  }

  // Create window
  _window = SDL_CreateWindow(cfg.name.c_str(), initWindowX, initWindowY,
                             _windowRect.w, _windowRect.h,
                             getValue(cfg.displayMode, cfg.borderMode));

  if (nullptr == _window) {
    LOGERR("Window could not be created! SDL Error: %s", SDL_GetError());
    return ErrorCode::FAILURE;
  }

  // obtain real window coordinates after creation
  SDL_GetWindowPosition(_window, &_windowRect.x, &_windowRect.y);

  Texture::setMonitorRect(_windowRect);
  LoadingScreen::setMonitorRect(_windowRect);

  if (!cfg.iconPath.empty()) {
    if (ErrorCode::SUCCESS != loadWindowIcon(cfg.iconPath)) {
      LOGERR("loadWindowIcon() failed!");
      return ErrorCode::FAILURE;
    }
  }

  return ErrorCode::SUCCESS;
}

void MonitorWindow::deinit() {
  if (_window) { // sanity check
    SDL_DestroyWindow(_window);
    _window = nullptr;
  }
}

ErrorCode MonitorWindow::loadWindowIcon(const std::string& iconPath) {
  SDL_Surface* windowIcon = IMG_Load(iconPath.c_str());
  if (nullptr == windowIcon) {
    LOGERR("Unable to create window Image from file! SDL Error: %s",
           SDL_GetError());
    return ErrorCode::FAILURE;
  }

  SDL_SetWindowIcon(_window, windowIcon);
  SDL_FreeSurface(windowIcon);
  windowIcon = nullptr;
  return ErrorCode::SUCCESS;
}

