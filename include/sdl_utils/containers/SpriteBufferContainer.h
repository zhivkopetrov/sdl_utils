#ifndef SDL_UTILS_SPRITEBUFFERCONTAINER_H_
#define SDL_UTILS_SPRITEBUFFERCONTAINER_H_

// C system headers

// C++ system headers
#include <cstdint>

// Other libraries headers
#include "sdl_utils/drawing/RendererDefines.h"

// Own components headers

// Forward declarations
class Renderer;
class Color;
struct SDL_Surface;
struct SDL_Texture;

class SpriteBufferContainer {
 public:
  SpriteBufferContainer();

  /** @brief used to initialise the SpriteBufferContainer
   *
   *  @return int32_t - error code
   * */
  int32_t init();

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

  /** @brief used to allocate memory for new empty surface/texture
   *         NOTE: this function does not return error code
   *                                              for performance reasons
   *
   *  @param const int32_t - width for the generated Texture/Surface
   *  @param const int32_t - height for the generated Texture/Surface
   *  @param int32_t &     - out unique container Id (used to determine
   *                                       unique _spriteBufferVec index)
   * */
  void createSpriteBuffer(const int32_t width, const int32_t height,
                          int32_t& outContainerId);

  /** @brief used to deallocate memory for selected sprite buffer's
   *                                                      surface/texture
   *
   *  @param const int32_t - unique container Id
   *                    (used to determine unique _spriteBufferVec index)
   * */
  void destroySpriteBuffer(const int32_t uniqueContainerId);

#if USE_SOFTWARE_RENDERER
  /** @brief used to attach a newly generated SDL_Surface by the renderer
   *         to the SpriteBufferContainer and increase the used GPU VRAM
   *
   *  @param const int32_t - uniqueContainerId
   *  @param const int32_t - created width of the SDL_Surface
   *  @param const int32_t - created height of the SDL_Surface
   *  @param SDL_Surface * - pointer to memory of the created SDL_Surface
   **/
  void attachSpriteBuffer(const int32_t containerId, const int32_t createdWidth,
                          const int32_t createdHeight,
                          SDL_Surface* createdTexture);

  /** @brief used to acquire previously stored pre-created SDL_Surface
   *                                       for a given unique resource ID
   *  This function does not return error code for performance reasons
   *
   *  @param const int32_t  - unique text resource ID
   *  @param SDL_Surface *& - pre-created SDL_Surface
   * */
  void getSpriteBufferTexture(const int32_t uniqueId, SDL_Surface*& outTexture);
#else
  /** @brief used to attach a newly generated SDL_Texture by the renderer
   *         to the SpriteBufferContainer and increase the used GPU VRAM
   *
   *  @param const int32_t - uniqueContainerId
   *  @param const int32_t - created width of the SDL_Texture
   *  @param const int32_t - created height of the SDL_Texture
   *  @param SDL_Texture * - pointer to memory of the created SDL_Texture
   **/
  void attachSpriteBuffer(const int32_t containerId, const int32_t createdWidth,
                          const int32_t createdHeight,
                          SDL_Texture* createdTexture);

  /** @brief used to acquire previously stored pre-created SDL_Texture
   *                                       for a given unique resource ID
   *  This function does not return error code for performance reasons
   *
   *  @param const int32_t  - unique text resource ID
   *  @param SDL_Texture *& - pre-created SDL_Texture
   * */
  void getSpriteBufferTexture(const int32_t uniqueId, SDL_Texture*& outTexture);
#endif /* USE_SOFTWARE_RENDERER */

  /** @brief used to detach(free the slot in the container) for
   *         successfully destroyed SDL_Surface/SDL_Texture by the
   *                              renderer and decrease the used GPU VRAM
   *
   *  @param const int32_t - uniqueContainerId
   **/
  void detachSpriteBuffer(const int32_t containerId);

  /** @brief used to acquire the occupied GPU VRAM from
   *                                            the SpriteBufferContainer
   *
   *  @return uint64_t - occupied VRAM in bytes
   * */
  inline uint64_t getGPUMemoryUsage() const { return _gpuMemoryUsage; }

 private:
  // holds pointer to hardware render in order
  // to be able to push RendererCmd's
  Renderer* _renderer;

  /** Raw array is used to store the Sprite Buffer Texture pointers and
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
  //_textsVec holds all Texts
  SDL_Surface* _spriteBuffers[RendererDefines::MAX_REAL_TIME_VBO_COUNT];
#else
  //_textsVec holds all Texts
  SDL_Texture* _spriteBuffers[RendererDefines::MAX_REAL_TIME_VBO_COUNT];

  // holds holds many bytes the current text occupied in GPU VRAM
  uint64_t _sbMemoryUsage[RendererDefines::MAX_REAL_TIME_VBO_COUNT];
#endif /* USE_SOFTWARE_RENDERER */

  // holds the currently occupied GPU VRAM in bytes
  uint64_t _gpuMemoryUsage;
};

#endif /* SDL_UTILS_SPRITEBUFFERCONTAINER_H_ */
