#ifndef SDL_UTILS_RENDERERCONFIG_H_
#define SDL_UTILS_RENDERERCONFIG_H_

// System headers
#include <cstdint>

// Other libraries headers

// Own components headers

// Forward declarations
struct SDL_Window;

struct RendererConfig {
  SDL_Window *window = nullptr;
  uint32_t maxRuntimeWidgets = 0;
  uint32_t maxRuntimeRendererCommands = 0;

  /** A buffer that holds all draw specific data populated by
   *  the main(update) thread
   **/
  uint64_t maxRendererBackBufferDataSize = 0;
};



#endif /* SDL_UTILS_RENDERERCONFIG_H_ */
