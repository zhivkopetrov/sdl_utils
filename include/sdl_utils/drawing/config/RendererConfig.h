#ifndef SDL_UTILS_RENDERERCONFIG_H_
#define SDL_UTILS_RENDERERCONFIG_H_

// System headers
#include <cstdint>

// Other libraries headers

// Own components headers
#include "sdl_utils/drawing/defines/RendererDefines.h"

// Forward declarations
struct SDL_Window;
struct SDL_Renderer;

struct RendererConfig {
  SDL_Window *window = nullptr;
  RendererPolicy executionPolicy = RendererPolicy::MULTI_THREADED;
  RendererScaleQuality scaleQuality = RendererScaleQuality::LINEAR;
  RendererFlagsMask flagsMask = 0;

  uint32_t maxRuntimeWidgets = 0;
  uint32_t maxRuntimeRendererCommands = 0;

  /** A buffer that holds all draw specific data populated by
   *  the main(update) thread
   **/
  uint64_t maxRendererBackBufferDataSize = 0;
};

bool isRendererFlagEnabled(RendererFlagsMask mask, RendererFlag flag);

void printRendererInfo(SDL_Renderer* renderer);

#endif /* SDL_UTILS_RENDERERCONFIG_H_ */
