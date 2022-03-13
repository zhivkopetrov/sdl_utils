#ifndef SDL_UTILS_LOADINGSCREENCONFIG_H_
#define SDL_UTILS_LOADINGSCREENCONFIG_H_

// System headers
#include <cstdint>
#include <string>

// Other libraries headers

// Own components headers

// Forward declarations

enum class LoadingScreenUsage : uint8_t {
  ENABLED,
  DISABLED
};

struct LoadingScreenConfig {
  std::string backgroundImagePath;
  std::string progressBarOnImagePath;
  std::string progressBarOffImagePath;
  LoadingScreenUsage loadingScreenUsage = LoadingScreenUsage::DISABLED;
};



#endif /* SDL_UTILS_LOADINGSCREENCONFIG_H_ */
