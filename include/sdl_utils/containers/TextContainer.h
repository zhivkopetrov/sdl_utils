#ifndef SDL_UTILS_TEXTCONTAINER_H_
#define SDL_UTILS_TEXTCONTAINER_H_

// C system headers

// C++ system headers
#include <cstdint>
#include <unordered_map>

// Other libraries headers
#include "sdl_utils/drawing/RendererDefines.h"

// Own components headers

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
   *
   *  @return int32_t       - error code
   * */
  int32_t init(std::unordered_map<uint64_t, TTF_Font *> *fontsContainer);

  /** @brief used to deinitialize (free memory occupied by Text container)
   * */
  void deinit();

  /** @brief used to acquire the global renderer (for pushing draw commands)
   *
   *  @return int32_t - error code
   * */
  inline void setRenderer(Renderer * renderer) {
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
   *  @returns int32_t      - error code
   * */
  void loadText(const uint64_t fontId, const char *text, const Color &color,
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

#if USE_SOFTWARE_RENDERER
  /** @brief used to attach a newly generated SDL_Surface by the renderer
   *         to the TextContainer and increase the used GPU VRAM
   *
   *  @param const int32_t - uniqueContainerId
   *  @param const int32_t - created width of the SDL_Surface
   *  @param const int32_t - created height of the SDL_Surface
   *  @param SDL_Surface * - pointer to memory of the created SDL_Surface
   **/
  void attachText(const int32_t containerId, const int32_t createdWidth,
                  const int32_t createdHeight, SDL_Surface *createdTexture);

  /** @brief used to acquire previously stored pre-created SDL_Surface
   *                                       for a given unique resource ID
   *  This function does not return error code for performance reasons
   *
   *  @param const int32_t  - unique text resource ID
   *  @param SDL_Surface *& - pre-created SDL_Surface
   * */
  void getTextTexture(const int32_t uniqueId, SDL_Surface *&outTexture);
#else
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
#endif /* USE_SOFTWARE_RENDERER */

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
  inline uint64_t getGPUMemoryUsage() const { return _gpuMemoryUsage; }

 private:
  // holds pointer to hardware render in order
  // to be able to push RendererCmd's
  Renderer *_renderer;

  /** Raw array is used to store the text Texture pointers and
   * their respective GPU VRAM usage for several reasons:
   *      > In order to maintain O(1) lookup speed;
   *      > For better cache performance;
   *      > Simultaneous read/write from different threads on
   *        DIFFERENT(guaranteed) indexes may occur.
   *        While using a raw array - this is fine.
   *        If we were to use a std::vector<> this would cause a crash,
   *        because it is prohibited by the standard.
   *
   * NOTE: When text is deleted - do not remove it from the structure,
   *       because this will be quite inefficient.
   *       Instead - simply leave the vector element with nullptr value.
   *       When new text is added - simply linearly scan the array and
   *       search for a free position (nullptr value).
   **/
#if USE_SOFTWARE_RENDERER
  SDL_Surface *_texts[RendererDefines::MAX_REAL_TIME_TEXT_COUNT];
#else
  SDL_Texture *_texts[RendererDefines::MAX_REAL_TIME_TEXT_COUNT];

  // holds holds many bytes the current text occupied in GPU VRAM
  uint64_t _textMemoryUsage[RendererDefines::MAX_REAL_TIME_TEXT_COUNT];
#endif /* USE_SOFTWARE_RENDERER */

  // a reference to the fonts container (used for text creation)
  std::unordered_map<uint64_t, TTF_Font *> *_fontsMapPtr;

  // holds the currently occupied GPU VRAM in bytes
  uint64_t _gpuMemoryUsage;
};

#endif /* SDL_UTILS_TEXTCONTAINER_H_ */
