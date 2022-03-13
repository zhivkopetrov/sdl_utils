/*
 * SDLContainers.h
 *
 *  @brief a class used to store SDL related containers for:
 *          > Images/Sprites;
 *          > Texts;
 *          > Fonts;
 *          > Sounds (Music + sound chunks);
 *
 *  NOTE: Images and Texts are logically separated, because Images have static
 *        behavior. Once loaded in memory, they stay there unless modified.
 *        Texts on the other hand are meant to be constantly changed.
 *        Their resources need to be freed and recreated every time their
 *        text content is changed
 *
 *
 * NOTE2: for cleaner access and for performance reasons(since they will be
 *        heavily used) - no setters/getters will be used for the containers.
 *        Instead they are inherited so users of SDLContainers can access
 *        their public methods.
 *
 * NOTE3: Heavy compilation headers such as
 *        <vector> and <unordered_map> will be included in the header.
 *        Be careful when you insert this header include into other header files
 */

#ifndef SDL_UTILS_SDLCONTAINERS_H_
#define SDL_UTILS_SDLCONTAINERS_H_

// System headers
#include <cstdint>

// Other libraries headers
#include "utils/ErrorCode.h"

// Own components headers
#include "sdl_utils/containers/FontContainer.h"
#include "sdl_utils/containers/ResourceContainer.h"
#include "sdl_utils/containers/SoundContainer.h"
#include "sdl_utils/containers/FboContainer.h"
#include "sdl_utils/containers/TextContainer.h"
#include "sdl_utils/containers/config/SDLContainersConfig.h"

// Forward declarations
class ResourceLoader;

class SDLContainers : public ResourceContainer,
                      public TextContainer,
                      public FontContainer,
                      public SoundContainer,
                      public FboContainer {
 public:
  explicit SDLContainers(const SDLContainersConfig &cfg);

  /** @brief used to initialise the SDL containers
   *
   *  @return ErrorCode - error code
   * */
  ErrorCode init();

  /** @brief used to deinitialize (free memory occupied by SDL containers)
   * */
  void deinit();

  /** @brief used to acquire the global renderer (for pushing draw commands)
   *
   *  @return int32_t - error code
   * */
  void setRenderer(Renderer * renderer);

 private:
  /** @brief used to load initiate all SDL containers at program start up
   *
   *  @returns ErrorCode - error code
   * */
  ErrorCode populateSDLContainers(ResourceLoader &rsrcLoader);

  SDLContainersConfig _config;
};

#endif /* SDL_UTILS_SDLCONTAINERS_H_ */
