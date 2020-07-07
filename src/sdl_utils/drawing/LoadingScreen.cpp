// Corresponding header
#include "sdl_utils/drawing/LoadingScreen.h"

// C system headers

// C++ system headers
#include <cstdlib>
#include <string>

// Other libraries headers
#include <SDL_render.h>

// Own components headers
#include "sdl_utils/drawing/Texture.h"
#include "sdl_utils/drawing/RendererDefines.h"
#include "sdl_utils/drawing/config/LoadingScreenConfig.hpp"
#include "utils/drawing/Color.h"
#include "utils/Log.h"

SDL_Renderer* LoadingScreen::_renderer = nullptr;

SDL_Texture* LoadingScreen::_loadingBackground = nullptr;

SDL_Texture* LoadingScreen::_progressBarOn = nullptr;

SDL_Texture* LoadingScreen::_progressBarOff = nullptr;

int32_t LoadingScreen::_totalFileSize = 0;

int32_t LoadingScreen::_currLoadedFileSize = 0;

int32_t LoadingScreen::_lastLoadedPercent = 0;

int32_t LoadingScreen::_monitorWidth = 0;
int32_t LoadingScreen::_monitorHeight = 0;

#define LOAD_WITH_PROGRESS_BAR 1

#define USE_LOADING_BACKGROUND_IMAGE 1

int32_t LoadingScreen::init(const LoadingScreenConfig &cfg,
                            const int32_t totalFileSize) {
#if USE_SOFTWARE_RENDERER && LOAD_WITH_PROGRESS_BAR
  LOGR(
      "Warning, Loading screen and loading progress bar "
      "are not supported for Software renderer!");

  return EXIT_SUCCESS;
#endif /* USE_SOFTWARE_RENDERER && LOAD_WITH_PROGRESS_BAR */
  _totalFileSize = totalFileSize;
  _monitorWidth = cfg.monitorWidth;
  _monitorHeight = cfg.monitorHeight;

  SDL_Surface* surface = nullptr;

#if USE_LOADING_BACKGROUND_IMAGE
  if (EXIT_SUCCESS !=
      Texture::loadSurfaceFromFile(cfg.backgroundImagePath.c_str(), surface)) {
    LOGERR("Error, could not load _loadingBackground Surface");
    return EXIT_FAILURE;
  }

  if (EXIT_SUCCESS !=
      Texture::loadTextureFromSurface(surface, _loadingBackground)) {
    LOGERR("Error, could not load _loadingBackground Texture");
    return EXIT_FAILURE;
  }
#endif /* USE_LOADING_BACKGROUND_IMAGE */

  if (EXIT_SUCCESS != Texture::loadSurfaceFromFile(
        cfg.progressBarOnImagePath.c_str(), surface)) {
    LOGERR("Error, could not load _progressBarOn Surface");
    return EXIT_FAILURE;
  }

  if (EXIT_SUCCESS !=
      Texture::loadTextureFromSurface(surface, _progressBarOn)) {
    LOGERR("Error, could not load _progressBarOn Texture");
    return EXIT_FAILURE;
  }

  if (EXIT_SUCCESS != Texture::loadSurfaceFromFile(
      cfg.progressBarOffImagePath.c_str(), surface)) {
    LOGERR("Error, could not load _progressBarOff Surface");
    return EXIT_FAILURE;
  }

  if (EXIT_SUCCESS !=
      Texture::loadTextureFromSurface(surface, _progressBarOff)) {
    LOGERR("Error, could not load _progressBarOff Texture");
    return EXIT_FAILURE;
  }

  // do an initial draw for zero loaded resources
  LoadingScreen::draw(0);  // 0 % loaded

  return EXIT_SUCCESS;
}

void LoadingScreen::deinit() {
  // Loading screen and loading progress bar
  // are not supported for Software renderer!
#if USE_SOFTWARE_RENDERER && LOAD_WITH_PROGRESS_BAR
  return;
#endif /* USE_SOFTWARE_RENDERER && LOAD_WITH_PROGRESS_BAR */

  _renderer = nullptr;

#if USE_LOADING_BACKGROUND_IMAGE
  Texture::freeTexture(_loadingBackground);
#endif /* USE_LOADING_BACKGROUND_IMAGE */

  Texture::freeTexture(_progressBarOn);
  Texture::freeTexture(_progressBarOff);
}

void LoadingScreen::onNewResourceLoaded(const int32_t loadedSize) {
  // Loading screen and loading progress bar
  // are not supported for Software renderer!
#if USE_SOFTWARE_RENDERER && LOAD_WITH_PROGRESS_BAR
  return;
#endif /* USE_SOFTWARE_RENDERER && LOAD_WITH_PROGRESS_BAR */

  _currLoadedFileSize += loadedSize;

  const int32_t currLoadedPercent = static_cast<int32_t>(
      (static_cast<double>(_currLoadedFileSize) / _totalFileSize) * 100);

  if (currLoadedPercent != _lastLoadedPercent) {
    _lastLoadedPercent = currLoadedPercent;
    LoadingScreen::draw(_lastLoadedPercent);
  }
}

void LoadingScreen::draw(const int32_t percentLoaded) {
  // clear screen
  if (EXIT_SUCCESS != SDL_RenderClear(_renderer)) {
    LOGERR("Error in, SDL_RenderClear(), SDL Error: %s", SDL_GetError());

    return;
  }

#if USE_LOADING_BACKGROUND_IMAGE
  const SDL_Rect backgroundRenderQuad { 0, 0, _monitorWidth, _monitorHeight };

  // Render to screen
  if (EXIT_SUCCESS !=
      SDL_RenderCopyEx(_renderer,           // the hardware renderer
                       _loadingBackground,  // source texture
                       nullptr,  // source rectangle (nullptr for whole rect)
                       &backgroundRenderQuad,  // destination rectangle
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
  SDL_Rect progressBarRenderQuad = { 710,          // progress bar start X
                                     200,          // progress bar end X
                                     LOADED_WIDHT, // destination width
                                     60 };         // destination height

  // Render to screen loaded percentage
  if (EXIT_SUCCESS !=
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
  if (EXIT_SUCCESS !=
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
