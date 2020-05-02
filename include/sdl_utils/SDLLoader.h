#ifndef SDL_UTILS_SDLLOADER_H_
#define SDL_UTILS_SDLLOADER_H_

// C system headers

// C++ system headers
#include <cstdint>

// Other libraries headers

// Own components headers

// Forward declarations

class SDLLoader {
 public:
  SDLLoader() = delete;

  /** @brief used to initialise external SDL sub-systems and
   *         SDL Containers (for Images/Sprites/Texts/Sounds/Musics)
   *
   *  @returns int32_t - error code
   * */
  static int32_t init();

  /** @brief used to deinitialse all external SDL sub-systems and
   *         SDL Containers (for Images/Sprites/Texts/Sounds/Musics)
   * */
  static void deinit();
};

#endif /* SDL_UTILS_SDLLOADER_H_ */
