// Corresponding header
#include "sdl_utils/containers/TextContainer.h"

// C system headers

// C++ system headers
#include <cstdlib>
#include <cstring>

// Other libraries headers

// Own components headers
#include "sdl_utils/drawing/Renderer.h"
#include "sdl_utils/drawing/Texture.h"
#include "utils/drawing/Color.h"
#include "utils/LimitValues.hpp"
#include "utils/Log.h"

// basically anything different than nullptr
#if USE_SOFTWARE_RENDERER
#define RESERVE_SLOT_VALUE (reinterpret_cast<SDL_Surface *>(1))
#else
#define RESERVE_SLOT_VALUE (reinterpret_cast<SDL_Texture *>(1))
#endif /* USE_SOFTWARE_RENDERER */

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
#if !USE_SOFTWARE_RENDERER
  _textMemoryUsage.resize(maxRuntimeTexts, 0);
#endif /* !USE_SOFTWARE_RENDERER */

  return EXIT_SUCCESS;
}

void TextContainer::deinit() {
  // release the reference to the fonts data map
  _fontsMapPtr = nullptr;

  for (int32_t i = 0; i < _textsSize; ++i) {
    // free index found
    if (nullptr != _texts[i]) {
#if USE_SOFTWARE_RENDERER
      Texture::freeSurface(_texts[i]);
#else
      Texture::freeTexture(_texts[i]);
#endif /* USE_SOFTWARE_RENDERER */
    }
  }

#if !USE_SOFTWARE_RENDERER
  _textMemoryUsage.clear();
#endif //!USE_SOFTWARE_RENDERER
}

void TextContainer::loadText(const uint64_t fontId, const char *text,
                             const Color &color, int32_t &outUniqueId,
                             int32_t &outTextWidth, int32_t &outTextHeight) {
  if (EXIT_SUCCESS != Texture::getTextDimensions(text, (*_fontsMapPtr)[fontId],
                                                 outTextWidth, outTextHeight)) {
    LOGERR("Error in getTextDimensions() for fontId: %#16lX", fontId);

    return;
  }

  bool foundIdx = false;
  int32_t chosenIndex = INIT_INT32_VALUE;

  for (int32_t i = 0; i < _textsSize; ++i) {
    // free index found, occupy it
    if (nullptr == _texts[i]) {
      foundIdx = true;
      chosenIndex = i;
      break;
    }
  }

#ifndef NDEBUG
  if (!foundIdx) {
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
  if (EXIT_SUCCESS != Texture::getTextDimensions(text, (*_fontsMapPtr)[fontId],
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

#if USE_SOFTWARE_RENDERER
void TextContainer::attachText(const int32_t containerId,
                               [[maybe_unused]]const int32_t createdWidth,
                               [[maybe_unused]]const int32_t createdHeight,
                               SDL_Surface *createdTexture)
#else
void TextContainer::attachText(const int32_t containerId,
                               const int32_t createdWidth,
                               const int32_t createdHeight,
                               SDL_Texture *createdTexture)
#endif /* USE_SOFTWARE_RENDERER */
{
  _texts[containerId] = createdTexture;

#if !USE_SOFTWARE_RENDERER
  // calculate how much GPU VRAM will be used
  _textMemoryUsage[containerId] =
      static_cast<uint64_t>((createdWidth * createdHeight * RGBA_BYTE_SIZE));

  // increase the occupied GPU memory usage counter for the new texture
  _gpuMemoryUsage += _textMemoryUsage[containerId];
#endif /* !!USE_SOFTWARE_RENDERER */
}

#if USE_SOFTWARE_RENDERER
void TextContainer::getTextTexture(const int32_t uniqueId,
                                   SDL_Surface *&outTexture)
#else
void TextContainer::getTextTexture(const int32_t uniqueId,
                                   SDL_Texture *&outTexture)
#endif /* USE_SOFTWARE_RENDERER */
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

#if !USE_SOFTWARE_RENDERER
  // decrease the occupied GPU memory usage counter for the old texture
  _gpuMemoryUsage -= _textMemoryUsage[containerId];

  _textMemoryUsage[containerId] = 0;
#endif /* USE_SOFTWARE_RENDERER */
}
