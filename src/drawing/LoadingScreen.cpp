// Corresponding header
#include "sdl_utils/drawing/LoadingScreen.h"

// C system headers

// C++ system headers
#include <string>

// Other libraries headers
#include <SDL_render.h>
#include "utils/drawing/Color.h"
#include "utils/ErrorCode.h"
#include "utils/Log.h"

// Own components headers
#include "sdl_utils/drawing/Texture.h"
#include "sdl_utils/drawing/defines/RendererDefines.h"
#include "sdl_utils/drawing/config/LoadingScreenConfig.h"

SDL_Renderer* LoadingScreen::_renderer = nullptr;

SDL_Texture* LoadingScreen::_loadingBackground = nullptr;

SDL_Texture* LoadingScreen::_progressBarOn = nullptr;

SDL_Texture* LoadingScreen::_progressBarOff = nullptr;

int32_t LoadingScreen::_totalFileSize = 0;

int32_t LoadingScreen::_currLoadedFileSize = 0;

int32_t LoadingScreen::_lastLoadedPercent = 0;

Rectangle LoadingScreen::_monitorRect;

bool LoadingScreen::_isUsed = false;

#define LOAD_WITH_PROGRESS_BAR 1

#define USE_LOADING_BACKGROUND_IMAGE 1

int32_t LoadingScreen::init(const LoadingScreenConfig &cfg,
                            const int32_t totalFileSize) {
  if (LoadingScreenUsage::DISABLED == cfg.loadingScreenUsage) {
    return SUCCESS;
  }

  _totalFileSize = totalFileSize;
  _isUsed = true;

  SDL_Surface* surface = nullptr;

#if USE_LOADING_BACKGROUND_IMAGE
  if (SUCCESS !=
      Texture::loadSurfaceFromFile(cfg.backgroundImagePath.c_str(), surface)) {
    LOGERR("Error, could not load _loadingBackground Surface");
    return FAILURE;
  }

  if (SUCCESS !=
      Texture::loadTextureFromSurface(surface, _loadingBackground)) {
    LOGERR("Error, could not load _loadingBackground Texture");
    return FAILURE;
  }
#endif /* USE_LOADING_BACKGROUND_IMAGE */

  if (SUCCESS != Texture::loadSurfaceFromFile(
        cfg.progressBarOnImagePath.c_str(), surface)) {
    LOGERR("Error, could not load _progressBarOn Surface");
    return FAILURE;
  }

  if (SUCCESS !=
      Texture::loadTextureFromSurface(surface, _progressBarOn)) {
    LOGERR("Error, could not load _progressBarOn Texture");
    return FAILURE;
  }

  if (SUCCESS != Texture::loadSurfaceFromFile(
      cfg.progressBarOffImagePath.c_str(), surface)) {
    LOGERR("Error, could not load _progressBarOff Surface");
    return FAILURE;
  }

  if (SUCCESS !=
      Texture::loadTextureFromSurface(surface, _progressBarOff)) {
    LOGERR("Error, could not load _progressBarOff Texture");
    return FAILURE;
  }

  // do an initial draw for zero loaded resources
  LoadingScreen::draw(0);  // 0 % loaded

  return SUCCESS;
}

void LoadingScreen::deinit() {
  if (!_isUsed) {
    return;
  }

  _renderer = nullptr;

#if USE_LOADING_BACKGROUND_IMAGE
  Texture::freeTexture(_loadingBackground);
#endif /* USE_LOADING_BACKGROUND_IMAGE */

  Texture::freeTexture(_progressBarOn);
  Texture::freeTexture(_progressBarOff);
}

void LoadingScreen::onNewResourceLoaded(const int32_t loadedSize) {
  if (!_isUsed) {
    return;
  }

  _currLoadedFileSize += loadedSize;

  const int32_t currLoadedPercent = static_cast<int32_t>(
      (static_cast<double>(_currLoadedFileSize) / _totalFileSize) * 100);

  if (currLoadedPercent != _lastLoadedPercent) {
    _lastLoadedPercent = currLoadedPercent;
    LoadingScreen::draw(_lastLoadedPercent);
  }
}

void LoadingScreen::setMonitorRect(const Rectangle& monitorRect) {
  //the X and Y should remain 0
  _monitorRect.w = monitorRect.w;
  _monitorRect.h = monitorRect.h;
}

void LoadingScreen::draw(const int32_t percentLoaded) {
  if (!_isUsed) {
    return;
  }

  // clear screen
  if (SUCCESS != SDL_RenderClear(_renderer)) {
    LOGERR("Error in, SDL_RenderClear(), SDL Error: %s", SDL_GetError());

    return;
  }



#if USE_LOADING_BACKGROUND_IMAGE
  const SDL_Rect* backgroundRenderQuad =
      reinterpret_cast<const SDL_Rect*>(&_monitorRect);

  // Render to screen
  if (SUCCESS !=
      SDL_RenderCopyEx(_renderer,           // the hardware renderer
                       _loadingBackground,  // source texture
                       nullptr,  // source rectangle (nullptr for whole rect)
                       backgroundRenderQuad,   // destination rectangle
                       0.0,                    // rotation angles
                       nullptr,                // rotation center
                       SDL_FLIP_NONE)) {       // flip mode
    LOGERR("Error in, SDL_RenderCopyEx(), SDL Error: %s", SDL_GetError());
    return;
  }
#endif /* USE_LOADING_BACKGROUND_IMAGE */

  constexpr int32_t OFFSET_X = 5;
  const int32_t LOADED_WIDHT = OFFSET_X * percentLoaded;
  // resize progress bar width to match the loaded resources percentage
  SDL_Rect progressBarRenderQuad = { 1150,         // progress bar start X
                                     300,          // progress bar end X
                                     LOADED_WIDHT, // destination width
                                     60 };         // destination height

  // Render to screen loaded percentage
  if (SUCCESS !=
      SDL_RenderCopyEx(_renderer,       // the hardware renderer
                       _progressBarOn,  // source texture
                       nullptr,  // source rectangle (nullptr for whole rect)
                       &progressBarRenderQuad,  // destination rectangle
                       0.0,                     // rotation angles
                       nullptr,                 // rotation center
                       SDL_FLIP_NONE)) {        // flip mode
    LOGERR("Error in, SDL_RenderCopyEx(), SDL Error: %s", SDL_GetError());
    return;
  }

  const int32_t REMAINING_WIDTH = (100 - percentLoaded) * OFFSET_X;

  // move progress bar starting position
  progressBarRenderQuad.x += LOADED_WIDHT;

  // resize progress bar width to match the remaining resources percentage
  progressBarRenderQuad.w = REMAINING_WIDTH;

  // Render to screen remaining percentage
  if (SUCCESS !=
      SDL_RenderCopyEx(_renderer,        // the hardware renderer
                       _progressBarOff,  // source texture
                       nullptr,  // source rectangle (nullptr for whole rect)
                       &progressBarRenderQuad,  // destination rectangle
                       0.0,                     // rotation angles
                       nullptr,                 // rotation center
                       SDL_FLIP_NONE)) {        // flip mode
    LOGERR("Error in, SDL_RenderCopyEx(), SDL Error: %s", SDL_GetError());
    return;
  }

  // update screen
  SDL_RenderPresent(_renderer);
}

void LoadingScreen::setRenderer(SDL_Renderer* renderer) {
  _renderer = renderer;
}
