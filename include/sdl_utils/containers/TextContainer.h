#ifndef SDL_UTILS_TEXTCONTAINER_H_
#define SDL_UTILS_TEXTCONTAINER_H_

// System headers
#include <cstdint>
#include <vector>
#include <unordered_map>

// Other libraries headers
#include "utils/ErrorCode.h"

// Own components headers
#include "sdl_utils/drawing/defines/RendererDefines.h"

// Forward declarations
class Renderer;
class Color;
struct SDL_Surface;
struct SDL_Texture;
typedef struct _TTF_Font TTF_Font;

class TextContainer {
 public:
  TextContainer();

  /** @brief used to initialise the Text container
   *
   *  @param std::unordered_map<uint64_t, TTF_Font *> * - reference to the
   *                                                      fonts Container
   *  @param const int32_t                              - max runtime texts
   *
   *  @return ErrorCode                                 - error code
   * */
  ErrorCode init(std::unordered_map<uint64_t, TTF_Font *> *fontsContainer,
               const int32_t maxRuntimeTexts);

  /** @brief used to deinitialize (free memory occupied by Text container)
   * */
  void deinit();

  /** @brief used to acquire the global renderer (for pushing draw commands)
   *
   *  @return int32_t - error code
   * */
   void setRenderer(Renderer * renderer) {
    _renderer = renderer;
  }

  /** @brief used to load text resource on demand
   *         NOTE: use this function when text is created for first time.
   *               If text re-creation is needed use reloadText(...);
   *         NOTE2: this function does not return error code
   *                                              for performance reasons
   *
   *  @param const uint64_t - unique font ID
   *  @param const char *   - text content
   *  @param const Color &  - text color
   *  @param int32_t &      - out unique Text Id (used to determine
   *                                              unique _textsVec index)
   *  @param int32_t &      - out width of the generated Texture/Surface
   *  @param int32_t &      - out height of the generated Texture/Surface
   *
   *  @returns ErrorCode    - error code
   * */
   ErrorCode loadText(
       const uint64_t fontId, const char *text, const Color &color,
       int32_t &outUniqueId, int32_t &outTextWidth,
       int32_t &outTextHeight);

  /** @brief used to reload text resource on demand on the SAME position
   *                in the _textsVec (new Text has the same uniqueTextId).
   *         NOTE: use this function when text is re-creating texts.
   *               If text needs to be created for first time use
   *                                                       loadText(...);
   *         NOTE2: this function does not return error code
   *                                              for performance reasons.
   *
   *  @param const uint64_t - unique font ID
   *  @param const char *   - text content
   *  @param const Color &  - text color
   *  @param int32_t        - text unique Id (used to determine
   *                                              unique _textsVec index)
   *  @param int32_t &      - out width of the generated Texture/Surface
   *  @param int32_t &      - out height of the generated Texture/Surface
   * */
  void reloadText(const uint64_t fontId, const char *text, const Color &color,
                  const int32_t textUniqueId, int32_t &outTextWidth,
                  int32_t &outTextHeight);

  /** @brief used to unload text resource on demand
   *
   *  @param const int32_t - unique Text Id
   *                           (used to determine unique _textsVec index)
   * */
  void unloadText(const int32_t textUniqueId);

  /** @brief used to attach a newly generated SDL_Texture by the renderer
   *         to the TextContainer and increase the used GPU VRAM
   *
   *  @param const int32_t - uniqueContainerId
   *  @param const int32_t - created width of the SDL_Texture
   *  @param const int32_t - created height of the SDL_Texture
   *  @param SDL_Texture * - pointer to memory of the created SDL_Texture
   **/
  void attachText(const int32_t containerId, const int32_t createdWidth,
                  const int32_t createdHeight, SDL_Texture *createdTexture);

  /** @brief used to acquire previously stored pre-created SDL_Texture
   *                                       for a given unique resource ID
   *  This function does not return error code for performance reasons
   *
   *  @param const int32_t  - unique text resource ID
   *  @param SDL_Texture *& - pre-created SDL_Texture
   * */
  void getTextTexture(const int32_t uniqueId, SDL_Texture *&outTexture);

  /** @brief used to detach(free the slot in the container) for
   *         successfully destroyed SDL_Surface/SDL_Texture by the
   *                              renderer and decrease the used GPU VRAM
   *
   *  @param const int32_t - uniqueContainerId
   **/
  void detachText(const int32_t containerId);

  /** @brief used to acquire the occupied GPU VRAM from
   *                                              the ResourceContainer
   *
   *  @return uint64_t - occupied VRAM in bytes
   * */
   uint64_t getGPUMemoryUsage() const { return _gpuMemoryUsage; }

 private:
  // holds pointer to hardware render in order
  // to be able to push RendererCmd's
  Renderer *_renderer;

  /** Store the text Texture pointers and
   * their respective GPU VRAM usage for several reasons:
   *      > In order to maintain O(1) lookup speed;
   *      > For better cache performance;
   *      > Simultaneous read/write from different threads on
   *        DIFFERENT(guaranteed) indexes may occur.
   *
   * NOTE: do NOT resize the vector. This will crash the program
   *
   * NOTE2: When text is deleted - do not remove it from the structure.
   *        Instead - simply leave the vector element with nullptr value.
   *        When new text is added - simply linearly scan the array and
   *        search for a free position (nullptr value).
   **/
  std::vector<SDL_Texture*> _texts;

  // holds holds many bytes the current text occupied in GPU VRAM
  std::vector<uint64_t> _textMemoryUsage;

  // a reference to the fonts container (used for text creation)
  std::unordered_map<uint64_t, TTF_Font *> *_fontsMapPtr;

  // holds the currently occupied GPU VRAM in bytes
  uint64_t _gpuMemoryUsage;

  // holds the _texts.size() count
  int32_t _textsSize;
};

#endif /* SDL_UTILS_TEXTCONTAINER_H_ */
