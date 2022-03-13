// Corresponding header
#include "sdl_utils/containers/FboContainer.h"

// System headers
#include <cstring>

// Other libraries headers
#include "utils/LimitValues.h"
#include "utils/drawing/Color.h"
#include "utils/Log.h"

// Own components headers
#include "sdl_utils/drawing/Renderer.h"
#include "sdl_utils/drawing/Texture.h"

// basically anything different than nullptr
#define RESERVE_SLOT_VALUE (reinterpret_cast<SDL_Texture *>(1))

#define RGBA_BYTE_SIZE 4

FboContainer::FboContainer()
  : _renderer(nullptr), _gpuMemoryUsage(0), _sbSize(0) {
}

ErrorCode FboContainer::init(const int32_t maxRuntimeSpriteBuffers) {
  _sbSize = maxRuntimeSpriteBuffers;
  _textures.resize(maxRuntimeSpriteBuffers, nullptr);
  _fboMemoryUsage.resize(maxRuntimeSpriteBuffers, 0);
  return ErrorCode::SUCCESS;
}

void FboContainer::deinit() {
  for (int32_t i = 0; i < _sbSize; ++i) {
    // free index found
    if ((nullptr != _textures[i]) &&
        (RESERVE_SLOT_VALUE != _textures[i])) {
      Texture::freeTexture(_textures[i]);
    }
  }

  _fboMemoryUsage.clear();
}

void FboContainer::createFbo(const int32_t width, const int32_t height,
                             int32_t &outContainerId) {
  int32_t chosenIndex = INIT_INT32_VALUE;

  for (int32_t i = 0; i < _sbSize; ++i) {
    // free index found, occupy it
    if (nullptr == _textures[i]) {
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

  _textures[chosenIndex] = RESERVE_SLOT_VALUE;
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

void FboContainer::destroyFbo(const int32_t uniqueContainerId) {
  // uniqueContainerId has default value -> is not set at all
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
void FboContainer::attachFbo(const int32_t containerId,
                             const int32_t createdWidth,
                             const int32_t createdHeight,
                             SDL_Texture *createdTexture)
{
  _textures[containerId] = createdTexture;

  // calculate how much GPU VRAM will be used
  _fboMemoryUsage[containerId] =
      static_cast<uint64_t>((createdWidth * createdHeight * RGBA_BYTE_SIZE));

  // increase the occupied GPU memory usage counter for the new texture
  _gpuMemoryUsage += _fboMemoryUsage[containerId];
}

void FboContainer::getFboTexture(const int32_t uniqueId,
                                 SDL_Texture *&outTexture)
{
  // sanity check - check if such index exists
  if (uniqueId < _sbSize) {
    outTexture = _textures[uniqueId];
  } else {
    LOGERR("Warning, trying to destroy sprite buffer with non-existent "
           "uniqueContainerId: %d", uniqueId);
  }
}

void FboContainer::detachFbo(const int32_t containerId) {
  _textures[containerId] = nullptr;

  // decrease the occupied GPU memory usage counter for the old texture
  _gpuMemoryUsage -= _fboMemoryUsage[containerId];

  _fboMemoryUsage[containerId] = 0;
}
