#ifndef SDL_UTILS_LOADINGSCREENCONFIG_HPP_
#define SDL_UTILS_LOADINGSCREENCONFIG_HPP_

// C system headers

// C++ system headers
#include <cstdint>
#include <string>

// Other libraries headers

// Own components headers

// Forward declarations

struct LoadingScreenConfig {
  std::string backgroundImagePath;
  std::string progressBarOnImagePath;
  std::string progressBarOffImagePath;
  int32_t monitorWidth = 0;
  int32_t monitorHeight = 0;
};



#endif /* SDL_UTILS_LOADINGSCREENCONFIG_HPP_ */
