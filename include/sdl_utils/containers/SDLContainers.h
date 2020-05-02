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

// C system headers

// C++ system headers
#include <cstdint>

// Other libraries headers

// Own components headers
#include "sdl_utils/containers/FontContainer.h"
#include "sdl_utils/containers/ResourceContainer.h"
#include "sdl_utils/containers/SoundContainer.h"
#include "sdl_utils/containers/SpriteBufferContainer.h"
#include "sdl_utils/containers/TextContainer.h"

// Forward declarations
class ResourceLoader;
class Renderer;

class SDLContainers : public ResourceContainer,
                      public TextContainer,
                      public FontContainer,
                      public SoundContainer,
                      public SpriteBufferContainer {
 public:
  explicit SDLContainers(Renderer* renderer, const std::string& projectName,
                         const bool isMultithreadResAllowed);

  /** @brief used to initialise the SDL containers
   *
   *  @return int32_t - error code
   * */
  int32_t init();

  /** @brief used to deinitialize (free memory occupied by SDL containers)
   * */
  void deinit();

 private:
  /** @brief used to load initiate all SDL containers at program start up
   *
   *  @returns int32_t - error code
   * */
  int32_t populateSDLContainers();

  // ResourceLoader is used to parse resource.bin and fonts.bin,
  // which were previously created by the resourceBuilder Tool
  ResourceLoader* _rsrcLoader;

  std::string _projectName;

  bool _isMultithreadResAllowed;
};

#endif /* SDL_UTILS_SDLCONTAINERS_H_ */
