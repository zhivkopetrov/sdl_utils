#ifndef SDL_UTILS_SDLCONTAINERSCONFIG_H_
#define SDL_UTILS_SDLCONTAINERSCONFIG_H_

//System headers

//Other libraries headers

//Own components headers
#include "sdl_utils/drawing/config/LoadingScreenConfig.h"

//Forward declarations

struct SDLContainersConfig {
  LoadingScreenConfig loadingScreenCfg;
  std::string resourcesFolderLocation;
  uint32_t maxResourceLoadingThreads = 0;
  int32_t maxRuntimeTexts = 0;
  int32_t maxRuntimeSpriteBuffers = 0;
};

#endif /* SDL_UTILS_INCLUDE_SDL_UTILS_CONTAINERS_CONFIG_SDLCONTAINERSCONFIG_H_ */
