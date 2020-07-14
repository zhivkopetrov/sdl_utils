// Corresponding header
#include "sdl_utils/containers/SpriteBufferContainer.h"

// C system headers

// C++ system headers
#include <cstdlib>
#include <cstring>

// Other libraries headers

// Own components headers
#include "sdl_utils/drawing/Renderer.h"
#include "sdl_utils/drawing/Texture.h"
#include "utils/LimitValues.hpp"
#include "utils/drawing/Color.h"
#include "utils/Log.h"

// basically anything different than nullptr
#if USE_SOFTWARE_RENDERER
#define RESERVE_SLOT_VALUE (reinterpret_cast<SDL_Surface *>(1))
#else
#define RESERVE_SLOT_VALUE (reinterpret_cast<SDL_Texture *>(1))
#endif /* USE_SOFTWARE_RENDERER */

#define RGBA_BYTE_SIZE 4

SpriteBufferContainer::SpriteBufferContainer()
  : _renderer(nullptr), _gpuMemoryUsage(0), _sbSize(0) {
}

int32_t SpriteBufferContainer::init(const int32_t maxRuntimeSpriteBuffers) {
  _sbSize = maxRuntimeSpriteBuffers;
  _spriteBuffers.resize(maxRuntimeSpriteBuffers, nullptr);
#if !USE_SOFTWARE_RENDERER
  _sbMemoryUsage.resize(maxRuntimeSpriteBuffers, 0);
#endif /* !USE_SOFTWARE_RENDERER */

  return EXIT_SUCCESS;
}

void SpriteBufferContainer::deinit() {
  for (int32_t i = 0; i < _sbSize; ++i) {
    // free index found
    if (nullptr != _spriteBuffers[i]) {
#if USE_SOFTWARE_RENDERER
      Texture::freeSurface(_spriteBuffers[i]);
#else
      Texture::freeTexture(_spriteBuffers[i]);
#endif /* USE_SOFTWARE_RENDERER */
    }
  }

#if !USE_SOFTWARE_RENDERER
  _sbMemoryUsage.clear();
#endif //!USE_SOFTWARE_RENDERER
}

void SpriteBufferContainer::createSpriteBuffer(const int32_t width,
                                               const int32_t height,
                                               int32_t &outContainerId) {
  int32_t chosenIndex = INIT_INT32_VALUE;

  for (int32_t i = 0; i < _sbSize; ++i) {
    // free index found, occupy it
    if (nullptr == _spriteBuffers[i]) {
      chosenIndex = i;
      break;
    }
  }

#ifndef NDEBUG
  if (INIT_INT32_VALUE == chosenIndex) {
    LOGERR("Critical Problem: maxRunTimeSpriteBuffers value: %d is reached! "
           "Increase it's value from the configuration! or reduce the number of"
           " active SpriteBuffers. SpriteBuffer will not be created in order "
           "to save the system from crashing", _sbSize);
    return;
  }
#endif //!NDEBUG

  _spriteBuffers[chosenIndex] = RESERVE_SLOT_VALUE;
  outContainerId = chosenIndex;

  uint8_t data[sizeof(width) + sizeof(height) + sizeof(chosenIndex)];
  uint64_t populatedBytes = 0;

  memcpy(data, &width, sizeof(width));
  populatedBytes += sizeof(width);

  memcpy(data + populatedBytes, &height, sizeof(height));
  populatedBytes += sizeof(height);

  memcpy(data + populatedBytes, &outContainerId, sizeof(chosenIndex));
  populatedBytes += sizeof(chosenIndex);

  _renderer->addRendererCmd_UT(RendererCmd::CREATE_FBO, data, populatedBytes);
}

void SpriteBufferContainer::destroySpriteBuffer(
    const int32_t uniqueContainerId) {
  // textUniqueId has default value -> is not set at all
  if (-1 == uniqueContainerId) {
    LOGERR("Warning, trying to destroy sprite buffer with non-existent "
           "uniqueContainerId: %d", uniqueContainerId);

    return;
  }

  if (uniqueContainerId >= _sbSize) {
    LOGERR("Critical Error, uniqueContainerId: %d is outside of text container "
           "size! There is an error in the internal business logic! Widget will"
           " not be destroyed in order to save the system from crashing.",
           uniqueContainerId);
    return;
  }

  _renderer->addRendererCmd_UT(
      RendererCmd::DESTROY_FBO,
      reinterpret_cast<const uint8_t *>(&uniqueContainerId),
      sizeof(uniqueContainerId));
}
#if USE_SOFTWARE_RENDERER
void SpriteBufferContainer::attachSpriteBuffer(
    const int32_t containerId,
    [[maybe_unused]]const int32_t createdWidth,
    [[maybe_unused]]const int32_t createdHeight,
    SDL_Surface *createdTexture)
#else
void SpriteBufferContainer::attachSpriteBuffer(const int32_t containerId,
                                               const int32_t createdWidth,
                                               const int32_t createdHeight,
                                               SDL_Texture *createdTexture)
#endif /* USE_SOFTWARE_RENDERER */
{
  _spriteBuffers[containerId] = createdTexture;

#if !USE_SOFTWARE_RENDERER
  // calculate how much GPU VRAM will be used
  _sbMemoryUsage[containerId] =
      static_cast<uint64_t>((createdWidth * createdHeight * RGBA_BYTE_SIZE));

  // increase the occupied GPU memory usage counter for the new texture
  _gpuMemoryUsage += _sbMemoryUsage[containerId];
#endif /* !USE_SOFTWARE_RENDERER */
}

#if USE_SOFTWARE_RENDERER
void SpriteBufferContainer::getSpriteBufferTexture(const int32_t uniqueId,
                                                   SDL_Surface *&outTexture)
#else
void SpriteBufferContainer::getSpriteBufferTexture(const int32_t uniqueId,
                                                   SDL_Texture *&outTexture)
#endif /* USE_SOFTWARE_RENDERER */
{
  // sanity check - check if such index exists
  if (uniqueId < _sbSize) {
    outTexture = _spriteBuffers[uniqueId];
  } else {
    LOGERR("Warning, trying to destroy sprite buffer with non-existent "
           "uniqueContainerId: %d", uniqueId);
  }
}

void SpriteBufferContainer::detachSpriteBuffer(const int32_t containerId) {
  _spriteBuffers[containerId] = nullptr;

#if !USE_SOFTWARE_RENDERER
  // decrease the occupied GPU memory usage counter for the old texture
  _gpuMemoryUsage -= _sbMemoryUsage[containerId];

  _sbMemoryUsage[containerId] = 0;
#endif /* USE_SOFTWARE_RENDERER */
}
