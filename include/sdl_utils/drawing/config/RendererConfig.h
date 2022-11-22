#ifndef SDL_UTILS_RENDERERCONFIG_H_
#define SDL_UTILS_RENDERERCONFIG_H_

// System headers
#include <cstdint>

// Other libraries headers

// Own components headers

// Forward declarations
struct SDL_Window;

enum class RendererPolicy {
  //execute rendering commands as part of the main (update thread)
  SINGLE_THREADED,

  //will occupy (block) the current (main) thread execute rendering commands
  //until shutdown
  MULTI_THREADED
};

struct RendererConfig {
  RendererPolicy executionPolicy = RendererPolicy::MULTI_THREADED;
  SDL_Window *window = nullptr;
  uint32_t maxRuntimeWidgets = 0;
  uint32_t maxRuntimeRendererCommands = 0;

  /** A buffer that holds all draw specific data populated by
   *  the main(update) thread
   **/
  uint64_t maxRendererBackBufferDataSize = 0;
};

#endif /* SDL_UTILS_RENDERERCONFIG_H_ */
