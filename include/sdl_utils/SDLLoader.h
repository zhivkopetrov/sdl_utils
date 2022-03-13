#ifndef SDL_UTILS_SDLLOADER_H_
#define SDL_UTILS_SDLLOADER_H_

// System headers
#include <cstdint>

// Other libraries headers
#include "utils/ErrorCode.h"

// Own components headers

// Forward declarations

class SDLLoader {
 public:
  SDLLoader() = delete;

  /** @brief used to initialise external SDL sub-systems and
   *         SDL Containers (for Images/Sprites/Texts/Sounds/Musics)
   *
   *  @returns ErrorCode - error code
   * */
  static ErrorCode init();

  /** @brief used to deinitialse all external SDL sub-systems and
   *         SDL Containers (for Images/Sprites/Texts/Sounds/Musics)
   * */
  static void deinit();
};

#endif /* SDL_UTILS_SDLLOADER_H_ */
