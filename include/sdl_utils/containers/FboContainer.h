#ifndef SDL_UTILS_FBOCONTAINER_H_
#define SDL_UTILS_FBOCONTAINER_H_

// System headers
#include <cstdint>
#include <vector>

// Other libraries headers
#include "utils/ErrorCode.h"

// Own components headers
#include "sdl_utils/drawing/defines/RendererDefines.h"

// Forward declarations
class Renderer;
class Color;
struct SDL_Surface;
struct SDL_Texture;

class FboContainer {
 public:
  FboContainer();

  /** @brief used to initialise the FboContainer
   *
   *  @param const int32_t - max runtime Sprite Buffers
   *
   *  @return ErrorCode    - error code
   * */
  ErrorCode init(const int32_t maxRuntimeSpriteBuffers);

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

  /** @brief used to allocate memory for new empty surface/texture
   *         NOTE: this function does not return error code
   *                                              for performance reasons
   *
   *  @param const int32_t - width for the generated Texture/Surface
   *  @param const int32_t - height for the generated Texture/Surface
   *  @param int32_t &     - out unique container Id (used to determine
   *                                       unique _fboVec index)
   * */
  void createFbo(const int32_t width, const int32_t height,
                 int32_t& outContainerId);

  /** @brief used to deallocate memory for selected sprite buffer's
   *                                                      surface/texture
   *
   *  @param const int32_t - unique container Id
   *                    (used to determine unique _fboVec index)
   * */
  void destroyFbo(const int32_t uniqueContainerId);

  /** @brief used to attach a newly generated SDL_Texture by the renderer
   *         to the SpriteBufferContainer and increase the used GPU VRAM
   *
   *  @param const int32_t - uniqueContainerId
   *  @param const int32_t - created width of the SDL_Texture
   *  @param const int32_t - created height of the SDL_Texture
   *  @param SDL_Texture * - pointer to memory of the created SDL_Texture
   **/
  void attachFbo(const int32_t containerId, const int32_t createdWidth,
                 const int32_t createdHeight, SDL_Texture* createdTexture);

  /** @brief used to acquire previously stored pre-created SDL_Texture
   *                                       for a given unique resource ID
   *  This function does not return error code for performance reasons
   *
   *  @param const int32_t  - unique text resource ID
   *  @param SDL_Texture *& - pre-created SDL_Texture
   * */
  void getFboTexture(const int32_t uniqueId, SDL_Texture*& outTexture);

  /** @brief used to detach(free the slot in the container) for
   *         successfully destroyed SDL_Surface/SDL_Texture by the
   *                              renderer and decrease the used GPU VRAM
   *
   *  @param const int32_t - uniqueContainerId
   **/
  void detachFbo(const int32_t containerId);

  /** @brief used to acquire the occupied GPU VRAM from the FboContainer
   *
   *  @return uint64_t - occupied VRAM in bytes
   * */
   uint64_t getGPUMemoryUsage() const { return _gpuMemoryUsage; }

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
   *
   * NOTE: do NOT resize the vector. This will crash the program
   *
   * NOTE2: When text is deleted - do not remove it from the structure.
   *        Instead - simply leave the vector element with nullptr value.
   *        When new text is added - simply linearly scan the array and
   *        search for a free position (nullptr value).
   **/
  //_textsVec holds all Texts
  std::vector<SDL_Texture*> _textures;

  // holds holds many bytes the current text occupied in GPU VRAM
  std::vector<uint64_t> _fboMemoryUsage;

  // holds the currently occupied GPU VRAM in bytes
  uint64_t _gpuMemoryUsage;

  // holds the _textures.size() count
  int32_t _sbSize;
};

#endif /* SDL_UTILS_FBOCONTAINER_H_ */
