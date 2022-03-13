// Corresponding header
#include "sdl_utils/drawing/Texture.h"

// System headers

// Other libraries headers
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "utils/drawing/Color.h"
#include "utils/Log.h"

// Own components headers
#include "sdl_utils/drawing/DrawParams.h"

SDL_Renderer *Texture::_renderer = nullptr;

Rectangle Texture::_monitorRect;

void Texture::freeSurface(SDL_Surface *&surface) {
  if (surface) { // sanity check
    SDL_FreeSurface(surface);
    surface = nullptr;
  }
}

void Texture::freeTexture(SDL_Texture *&texture) {
  if (texture) { // sanity check
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }
}

void Texture::setMonitorRect(const Rectangle &monitorRect) {
  //the X and Y should remain 0
  _monitorRect.w = monitorRect.w;
  _monitorRect.h = monitorRect.h;
}

ErrorCode Texture::getTextDimensions(const char *text, TTF_Font *font,
                                     int32_t &outTextWidth,
                                     int32_t &outTextHeight) {
  if (EXIT_SUCCESS !=
      TTF_SizeText(font, text, &outTextWidth, &outTextHeight)) {
    LOGERR("TTF_SizeText() failed! SDL_image Error: %s", IMG_GetError());
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}

ErrorCode Texture::loadSurfaceFromFile(const char *path,
                                     SDL_Surface *&outTexture) {
  // memory leak check
  if (nullptr != outTexture) {
    LOGERR("Warning non-nullptr detected! Will no create Surface. "
           "Memory leak prevented!");
    return ErrorCode::FAILURE;
  }

  // Load image at specified path
  outTexture = IMG_Load(path);
  if (nullptr == outTexture) {
    LOGERR("Unable to load image %s! SDL_image Error: %s", path,
           IMG_GetError());
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}

ErrorCode Texture::loadFromText(const char *text, TTF_Font *font,
                                const Color &color, SDL_Texture *&outTexture,
                                int32_t &outTextWidth, int32_t &outTextHeight)
{
  freeTexture(outTexture);

#if USE_ANTI_ALIASING_ON_TEXT
  SDL_Surface *loadedSurface = TTF_RenderText_Blended(font, text,
      *(reinterpret_cast<const SDL_Color *>(&color.rgba)));
#else
  SDL_Surface *loadedSurface = TTF_RenderText_Solid(
      font, text, *(reinterpret_cast<const SDL_Color *>(&color.rgba)));
#endif /* USE_ANTI_ALIASING_ON_TEXT */
  if (loadedSurface == nullptr) {
    LOGERR("Unable to load image! SDL_image Error: %s", IMG_GetError());
    return ErrorCode::FAILURE;
  }

  outTextWidth = loadedSurface->w;
  outTextHeight = loadedSurface->h;

  //create hardware accelerated texture
  if (ErrorCode::SUCCESS !=
      Texture::loadTextureFromSurface(loadedSurface, outTexture)) {
    LOGERR("Unable to create text texture");
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}

ErrorCode Texture::loadTextureFromSurface(
    SDL_Surface *&surface, SDL_Texture *&outTexture) {
  if (nullptr == surface) {
    LOGERR("Nullptr surface detected. Unable to loadFromSurface()");
    return ErrorCode::FAILURE;
  }

  // Check for memory leaks and get rid of preexisting texture
  if (outTexture) {
    LOGERR("Warning, Memory leak detected! Trying to load a new "
           "texture before calling delete on the old one");
    freeTexture(outTexture);
  }

  // Create texture from surface pixels
  outTexture = SDL_CreateTextureFromSurface(_renderer, surface);

  if (nullptr == outTexture) {
    LOGERR("Unable to create texture! SDL Error: %s", SDL_GetError());
    return ErrorCode::FAILURE;
  }

  // Get rid of old loaded surface
  freeSurface(surface);

  return ErrorCode::SUCCESS;
}

ErrorCode Texture::createEmptySurface(const int32_t width, const int32_t height,
                                      SDL_Surface *&outSurface) {
  if (nullptr != outSurface) {
    LOGERR("Warning, outSurface is not empty. Will not create Empty "
           "Surface. Memory leak prevented.");
    return ErrorCode::FAILURE;
  }

  /** If the depth is 4 or 8 bits, an empty palette will be allocated
   * for the surface.
   * If the depth is greater than 8 bits, the pixel format is set
   * using the flags '[RGB]/[RGBA] mask'.
   * */
  constexpr uint32_t DEPTH = 32;

  /** SDL interprets each pixel as a 32-bit number, so masks must depend
   * on the endianness (byte order) of the machine
   * */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  const uint32_t RED_MASK = 0xFF000000;
  const uint32_t GREEN_MASK = 0x00FF0000;
  const uint32_t BLUE_MASK = 0x0000FF00;
  const uint32_t ALPHA_MASK = 0x000000FF;
#else
  const uint32_t RED_MASK = 0xFF000000;
  const uint32_t GREEN_MASK = 0x00FF0000;
  const uint32_t BLUE_MASK = 0x0000FF00;
  const uint32_t ALPHA_MASK = 0x000000FF;
#endif /* SDL_BYTEORDER == SDL_BIG_ENDIAN */

  outSurface = SDL_CreateRGBSurface(0,            // deprecated SDL param
                                    width,        // surface width
                                    height,       // surface height
                                    DEPTH,        // surface depth (using)
                                    RED_MASK,     // surface red mask
                                    GREEN_MASK,   // surface green mask
                                    BLUE_MASK,    // surface blue mask
                                    ALPHA_MASK);  // surface alpha mask

  if (nullptr == outSurface) {
    LOGERR("SDL_CreateRGBSurface() failed: %s", SDL_GetError());
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}

ErrorCode Texture::createEmptyTexture(const int32_t width, const int32_t height,
                                      SDL_Texture *&outTexture) {
  if (nullptr != outTexture) {
    LOGERR("Warning, outTexture is not empty. Will not create Empty "
           "Surface. Memory leak prevented.");
    return ErrorCode::FAILURE;
  }

  /** NOTE: This method will probably be used by Hardware Accelerated
   *        SpriteBuffers, whose Texture's are going to be temporary
   *        targets for the Hardware Accelerated Renderer.
   *        In order for this to work, the SDL_Texture must be created
   *        with access specifier: SDL_TEXTUREACCESS_TARGET.
   *
   * NOTE2: For empty surface format - 32 bit depth format [RGBA] is used
   * */

  outTexture = SDL_CreateTexture(_renderer,  // hardware renderer
                                 SDL_PIXELFORMAT_RGBA8888,  // format
                                 SDL_TEXTUREACCESS_TARGET,  // access
                                 width,                     // texture width
                                 height);                   // texture height

  if (nullptr == outTexture) {
    LOGERR("SDL_CreateTexture() failed: %s", SDL_GetError());
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}

ErrorCode Texture::clearCurrentRendererTarget(const Color &clearColor) {
  Color currRendererColor = Colors::BLACK;
  bool isSameColorAsOld = true;

  // remember old renderer color
  if (EXIT_SUCCESS != SDL_GetRenderDrawColor(
                          _renderer, &currRendererColor.rgba.r,
                          &currRendererColor.rgba.g, &currRendererColor.rgba.b,
                          &currRendererColor.rgba.a)) {
    LOGERR("Error in, SDL_GetRenderDrawColor(), SDL Error: %s", SDL_GetError());
    return ErrorCode::FAILURE;
  }

  if (currRendererColor != clearColor) {
    isSameColorAsOld = false;
  }

  if (isSameColorAsOld) {
    // clear target
    if (EXIT_SUCCESS != SDL_RenderClear(_renderer)) {
      LOGERR("Error in, SDL_RenderClear(), SDL Error: %s", SDL_GetError());
      return ErrorCode::FAILURE;
    }
  } else { // color should be changed temporary
    // set new renderer color
    if (EXIT_SUCCESS != SDL_SetRenderDrawColor(
                            _renderer, clearColor.rgba.r, clearColor.rgba.g,
                            clearColor.rgba.b, clearColor.rgba.a)) {
      LOGERR("Error in, SDL_SetRenderDrawColor(), SDL Error: %s",
             SDL_GetError());
      return ErrorCode::FAILURE;
    }

    // clear target
    if (EXIT_SUCCESS != SDL_RenderClear(_renderer)) {
      LOGERR("Error in, SDL_RenderClear(), SDL Error: %s", SDL_GetError());
      return ErrorCode::FAILURE;
    }

    // restore old renderer color
    if (EXIT_SUCCESS != SDL_SetRenderDrawColor(
            _renderer, currRendererColor.rgba.r, currRendererColor.rgba.g,
            currRendererColor.rgba.b, currRendererColor.rgba.a)) {
      LOGERR("Error in, SDL_SetRenderDrawColor(), SDL Error: %s",
             SDL_GetError());
      return ErrorCode::FAILURE;
    }
  }

  return ErrorCode::SUCCESS;
}

ErrorCode Texture::setRendererTarget(SDL_Texture *target)
{
  if (nullptr == _renderer) {
    LOGERR("Error, renderer is still not set for Texture. You are missing "
           "Texture::setRenderer() call in the program initialization process");
    return ErrorCode::FAILURE;
  }

  if (EXIT_SUCCESS != SDL_SetRenderTarget(_renderer, target)) {
    LOGERR("Error, default renderer target could not be set. "
           "SDL_SetRenderTarget() failed, SDL Error: %s", SDL_GetError());
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}

void Texture::draw(SDL_Texture *texture, const DrawParams &drawParams)
{
  const SDL_Rect *SDLFrameRect =
      reinterpret_cast<const SDL_Rect *>(&drawParams.frameRect);

  SDL_Rect renderQuad = { 0, 0, 0, 0 };

  bool rendererClipped = false;

  if (drawParams.hasCrop) { // has crop and no scaling
    renderQuad = {drawParams.frameCropRect.x, drawParams.frameCropRect.y,
                  drawParams.frameCropRect.w, drawParams.frameCropRect.h};

    if (drawParams.hasScaling) { // has crop and scaling
      // Empty renderQuad rectangle -> do not make a render call
      // additional check is needed here, because scaledWidth/scaledHeight
      // will be overridden by the applyScaledCrop() calculations ->
      // therefore this will result in 0 values
      //(Rectangle::ZERO, crop with no common intersection)
      if (renderQuad.w == 0 || renderQuad.h == 0) {
        return;
      }

      if (drawParams.scaledWidth > drawParams.frameCropRect.w) {
        if (EXIT_SUCCESS != SDL_RenderSetClipRect(_renderer, &renderQuad)) {
          LOGERR("Error in SDL_RenderSetClipRect, SDL Error: %s",
                 SDL_GetError());
          return;
        }
        rendererClipped = true;
        renderQuad.w = drawParams.scaledWidth;
      }

      if (drawParams.scaledHeight > drawParams.frameCropRect.h) {
        if (!rendererClipped) {
          if (EXIT_SUCCESS != SDL_RenderSetClipRect(_renderer, &renderQuad)) {
            LOGERR("Error in SDL_RenderSetClipRect, SDL Error: %s",
                   SDL_GetError());
            return;
          }
          rendererClipped = true;
        }

        renderQuad.h = drawParams.scaledHeight;
      }
    }
  } else if (drawParams.hasScaling) { // has no crop and scaling
    renderQuad = {drawParams.pos.x, drawParams.pos.y, drawParams.scaledWidth,
                  drawParams.scaledHeight};
  } else { // has no crop and no scaling
    renderQuad = {drawParams.pos.x, drawParams.pos.y, drawParams.frameRect.w,
                  drawParams.frameRect.h};
  }

  // Empty renderQuad rectangle -> do not make a render call
  if (renderQuad.w == 0 || renderQuad.h == 0) {
    return;
  }

  // Render to screen
  if (EXIT_SUCCESS != SDL_RenderCopyEx(
      _renderer,         // the hardware renderer
      texture,           // source texture
      SDLFrameRect,      // source rectangle
      &renderQuad,       // destination rectangle
      drawParams.angle,  // rotation angles
      reinterpret_cast<const SDL_Point *>(
           &drawParams.rotCenter),  // rotation center
      static_cast<SDL_RendererFlip>(
          drawParams.widgetFlipType))) { // flip mode
    LOGERR("Error in SDL_RenderCopyEx(), SDL Error: %s from widget with "
           "rsrcId: %#16lX ", SDL_GetError(), drawParams.rsrcId);
    return;
  }

  // if clipping was done on the Hardware renderer -> reset it to default
  // monitor rectangle
  if (rendererClipped) {
    if (EXIT_SUCCESS != SDL_RenderSetClipRect(
            _renderer, reinterpret_cast<const SDL_Rect *>(&_monitorRect))) {
      LOGERR("Error in SDL_RenderSetClipRect(), SDL Error: %s", SDL_GetError());

      return;
    }
  }
}

void Texture::setRenderer(SDL_Renderer *renderer)
{
  _renderer = renderer;
}

void Texture::setAlpha(SDL_Texture *texture, const int32_t alpha) {
  if (EXIT_SUCCESS !=
      SDL_SetTextureAlphaMod(texture, static_cast<uint8_t>(alpha))) {
    LOGERR("Warning, .setAlpha() method will not take effect. Reason: "
           "invalid texture or alpha modulation is not supported. "
           "SDL_SetTextureAlphaMod() failed. SDL Error: %s", SDL_GetError());
    return;
  }
}

ErrorCode Texture::setBlendMode(SDL_Texture *texture,
                                const BlendMode blendMode) {
  if (EXIT_SUCCESS !=
      SDL_SetTextureBlendMode(texture, static_cast<SDL_BlendMode>(blendMode))) {
    LOGERR("Warning, .setBlendMode() method will not take effect. Reason: "
           "invalid texture or blend mode is not supported. "
           "SDL_SetTextureBlendMode() failed. SDL Error: %s", SDL_GetError());
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}
