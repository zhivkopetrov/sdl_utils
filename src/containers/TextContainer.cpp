// Corresponding header
#include "sdl_utils/containers/TextContainer.h"

// C system headers

// C++ system headers
#include <cstring>

// Other libraries headers

// Own components headers
#include "sdl_utils/drawing/Renderer.h"
#include "sdl_utils/drawing/Texture.h"
#include "utils/drawing/Color.h"
#include "utils/LimitValues.hpp"
#include "utils/ErrorCode.h"
#include "utils/Log.h"

// basically anything different than nullptr
#define RESERVE_SLOT_VALUE (reinterpret_cast<SDL_Texture *>(1))

#define RGBA_BYTE_SIZE 4

TextContainer::TextContainer()
    : _renderer(nullptr), _fontsMapPtr(nullptr), _gpuMemoryUsage(0),
      _textsSize(0) {
}

int32_t TextContainer::init(
    std::unordered_map<uint64_t, TTF_Font *> *fontsContainer,
    const int32_t maxRuntimeTexts) {
  _textsSize = maxRuntimeTexts;
  _fontsMapPtr = fontsContainer;
  _texts.resize(maxRuntimeTexts, nullptr);
  _textMemoryUsage.resize(maxRuntimeTexts, 0);

  return SUCCESS;
}

void TextContainer::deinit() {
  // release the reference to the fonts data map
  _fontsMapPtr = nullptr;

  for (int32_t i = 0; i < _textsSize; ++i) {
    // free index found
    if ((nullptr != _texts[i]) && ((RESERVE_SLOT_VALUE != _texts[i]))) {
      Texture::freeTexture(_texts[i]);
    }
  }

  _textMemoryUsage.clear();
}

void TextContainer::loadText(const uint64_t fontId, const char *text,
                             const Color &color, int32_t &outUniqueId,
                             int32_t &outTextWidth, int32_t &outTextHeight) {
  if (SUCCESS != Texture::getTextDimensions(text, (*_fontsMapPtr)[fontId],
                                                 outTextWidth, outTextHeight)) {
    LOGERR("Error in getTextDimensions() for fontId: %#16lX", fontId);

    return;
  }

  int32_t chosenIndex = INIT_INT32_VALUE;

  for (int32_t i = 0; i < _textsSize; ++i) {
    // free index found, occupy it
    if (nullptr == _texts[i]) {
      chosenIndex = i;
      break;
    }
  }

#ifndef NDEBUG
  if (INIT_INT32_VALUE == chosenIndex) {
    LOGERR("Critical Problem: maxRunTimeTexts value: %d is reached! "
           "Increase it's value from the configuration! or reduce the number of"
           " active texts. Text with content: %s will not be created in order "
           "to save the system from crashing", _textsSize, text);
    return;
  }
#endif //!NDEBUG

  _texts[chosenIndex] = RESERVE_SLOT_VALUE;
  outUniqueId = chosenIndex;

  const uint64_t TEXT_LEN = strlen(text);
  uint8_t data[sizeof(chosenIndex) + sizeof(fontId) + sizeof(color) +
               sizeof(TEXT_LEN) + TEXT_LEN];

  uint64_t populatedBytes = 0;

  memcpy(data, &chosenIndex, sizeof(chosenIndex));
  populatedBytes += sizeof(chosenIndex);

  memcpy(data + populatedBytes, &fontId, sizeof(fontId));
  populatedBytes += sizeof(fontId);

  memcpy(data + populatedBytes, &color, sizeof(color));
  populatedBytes += sizeof(color);

  memcpy(data + populatedBytes, &TEXT_LEN, sizeof(TEXT_LEN));
  populatedBytes += sizeof(TEXT_LEN);

  memcpy(data + populatedBytes, text, TEXT_LEN);
  populatedBytes += TEXT_LEN;

  _renderer->addRendererCmd_UT(RendererCmd::CREATE_TTF_TEXT, data,
                               populatedBytes);
}

void TextContainer::reloadText(const uint64_t fontId, const char *text,
                               const Color &color,
                               const int32_t textUniqueId,
                               int32_t &outTextWidth, int32_t &outTextHeight) {
  if (SUCCESS != Texture::getTextDimensions(text, (*_fontsMapPtr)[fontId],
                                                 outTextWidth, outTextHeight)) {
    LOGERR("Error in getTextDimensions() for fontId: %#16lX", fontId);
    return;
  }

  const uint64_t TEXT_LEN = strlen(text);
  uint8_t data[sizeof(textUniqueId) + sizeof(fontId) + sizeof(color) +
                sizeof(TEXT_LEN) + TEXT_LEN];

  uint64_t populatedBytes = 0;

  memcpy(data, &textUniqueId, sizeof(textUniqueId));
  populatedBytes += sizeof(textUniqueId);

  memcpy(data + populatedBytes, &fontId, sizeof(fontId));
  populatedBytes += sizeof(fontId);

  memcpy(data + populatedBytes, &color, sizeof(color));
  populatedBytes += sizeof(color);

  memcpy(data + populatedBytes, &TEXT_LEN, sizeof(TEXT_LEN));
  populatedBytes += sizeof(TEXT_LEN);

  memcpy(data + populatedBytes, text, TEXT_LEN);
  populatedBytes += TEXT_LEN;

  _renderer->addRendererCmd_UT(RendererCmd::RELOAD_TTF_TEXT, data,
                               populatedBytes);
}

void TextContainer::unloadText(const int32_t textUniqueId) {
  // textUniqueId has default value -> is not set at all
  if (-1 == textUniqueId) {
    LOGERR("Warning, trying to unload text with non-existent uniqueId: %d",
           textUniqueId);

    return;
  }

  if (textUniqueId >= _textsSize) {
    LOGERR("Critical Error, textUniqueId: %d is outside of text container size!"
           " There is an error in the internal business logic! Widget will not "
           "be destroyed in order to save the system from crashing.",
           textUniqueId);
    return;
  }

  _renderer->addRendererCmd_UT(RendererCmd::DESTROY_TTF_TEXT,
                               reinterpret_cast<const uint8_t *>(&textUniqueId),
                               sizeof(textUniqueId));
}

void TextContainer::attachText(const int32_t containerId,
                               const int32_t createdWidth,
                               const int32_t createdHeight,
                               SDL_Texture *createdTexture)
{
  _texts[containerId] = createdTexture;

  // calculate how much GPU VRAM will be used
  _textMemoryUsage[containerId] =
      static_cast<uint64_t>(createdWidth) * createdHeight * RGBA_BYTE_SIZE;

  // increase the occupied GPU memory usage counter for the new texture
  _gpuMemoryUsage += _textMemoryUsage[containerId];
}

void TextContainer::getTextTexture(const int32_t uniqueId,
                                   SDL_Texture *&outTexture)
{
  // sanity check - check if such index exists
  if (uniqueId < _textsSize) {
    outTexture = _texts[uniqueId];
  } else {
    LOGERR("Warning, trying to get text with non-existent uniqueId: %d",
           uniqueId);
  }
}

void TextContainer::detachText(const int32_t containerId) {
  _texts[containerId] = nullptr;

  // decrease the occupied GPU memory usage counter for the old texture
  _gpuMemoryUsage -= _textMemoryUsage[containerId];

  _textMemoryUsage[containerId] = 0;
}
