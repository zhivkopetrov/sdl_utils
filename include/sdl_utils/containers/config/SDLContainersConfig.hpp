#ifndef SDL_UTILS_SDLCONTAINERSCONFIG_HPP_
#define SDL_UTILS_SDLCONTAINERSCONFIG_HPP_

//C system headers

//C++ system headers

//Other libraries headers

//Own components headers
#include "sdl_utils/drawing/config/LoadingScreenConfig.hpp"

//Forward declarations

struct SDLContainersConfig {
  LoadingScreenConfig loadingScreenCfg;
  std::string resourcesBinLocation;
  int32_t maxRuntimeTexts = 0;
  int32_t maxRuntimeSpriteBuffers = 0;
  bool isMultithreadResAllowed = false;
};

#endif /* SDL_UTILS_INCLUDE_SDL_UTILS_CONTAINERS_CONFIG_SDLCONTAINERSCONFIG_HPP_ */
