// Corresponding header
#include "sdl_utils/drawing/Texture.h"

// C system headers

// C++ system headers

// Other libraries headers
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

// Own components headers
#if USE_SOFTWARE_RENDERER
#include "Camera.h"
#endif /* USE_SOFTWARE_RENDERER */

#include "sdl_utils/drawing/DrawParams.h"
#include "utils/drawing/Color.h"
#include "utils/Log.h"

#if USE_SOFTWARE_RENDERER
SDL_Surface *Texture::_renderer = nullptr;
#else
SDL_Renderer *Texture::_renderer = nullptr;

Rectangle *Texture::_monitorRect = nullptr;
#endif /* USE_SOFTWARE_RENDERER */

void Texture::freeSurface(SDL_Surface *&surface) {
  if (surface)  // sanity check
  {
    SDL_FreeSurface(surface);
    surface = nullptr;
  }
}

void Texture::freeTexture(SDL_Texture *&texture) {
  if (texture)  // sanity check
  {
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }
}

void Texture::setMonitorRect(Rectangle &monitorRect) {
#if USE_SOFTWARE_RENDERER
  // dummy check to satisfy Wunused-variable gcc warning
  if (monitorRect.x) {
  }

  LOGERR("Warning, setMonitorRect is not supported for Software renderer.");

  return;
#else
  _monitorRect = &monitorRect;
#endif /* USE_SOFTWARE_RENDERER */
}

int32_t Texture::getTextDimensions(const char *text, TTF_Font *font,
                                   int32_t *outTextWidth,
                                   int32_t *outTextHeight) {
  if (EXIT_SUCCESS != TTF_SizeText(font, text, outTextWidth, outTextHeight)) {
    LOGERR("TTF_SizeText() failed! SDL_image Error: %s", IMG_GetError());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int32_t Texture::loadSurfaceFromFile(const char *path,
                                     SDL_Surface *&outTexture) {
  // memory leak check
  if (nullptr != outTexture) {
    LOGERR(
        "Warning non-nullptr detected! Will no create Surface. "
        "Memory leak prevented!");
    return EXIT_FAILURE;
  }

  // Load image at specified path
  outTexture = IMG_Load(path);
  if (nullptr == outTexture) {
    LOGERR("Unable to load image %s! SDL_image Error: %s", path,
           IMG_GetError());

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

#if USE_SOFTWARE_RENDERER
int32_t Texture::loadFromText(const char *text,
                              TTF_Font *font,
                              const Color &color,
                              SDL_Surface *&outTexture,
                              int32_t *outTextWidth,
                              int32_t *outTextHeight)
#else
int32_t Texture::loadFromText(const char *text,
                              TTF_Font *font,
                              const Color &color,
                              SDL_Texture *&outTexture,
                              int32_t *outTextWidth,
                              int32_t *outTextHeight)
#endif /* USE_SOFTWARE_RENDERER */
{
  // free the existing texture
#if USE_SOFTWARE_RENDERER
  freeSurface(outTexture);
#else
  freeTexture(outTexture);
#endif /* USE_SOFTWARE_RENDERER */

#if USE_FAST_QUALITY_TEXT
  SDL_Surface *loadedSurface = TTF_RenderText_Solid(
      font, text, *(reinterpret_cast<const SDL_Color *>(&color.rgba)));
#else
  SDL_Surface *loadedSurface =
      TTF_RenderText_Blended(font, text,
          *(reinterpret_cast<const SDL_Color *>(&color.rgba)));
#endif
  if (loadedSurface == nullptr) {
    LOGERR("Unable to load image! SDL_image Error: %s", IMG_GetError());
    return EXIT_FAILURE;
  }

  *outTextWidth = loadedSurface->w;
  *outTextHeight = loadedSurface->h;

  //create hardware accelerated texture
  if (EXIT_SUCCESS
      != Texture::loadTextureFromSurface(loadedSurface, outTexture)) {
    LOGERR("Unable to create text texture");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int32_t Texture::loadTextureFromSurface(SDL_Surface *&surface,
                                        SDL_Texture *&outTexture) {
  if (nullptr == surface) {
    LOGERR("Nullptr surface detected. Unable to loadFromSurface()");
    return EXIT_FAILURE;
  }

#if USE_SOFTWARE_RENDERER
    LOGERR(
        "Error, loadTextureFromSurface is designed to be used by"
        "hardware accelerated Texture. "
        "You can not them with software renderer");
    err = EXIT_FAILURE;

    // dummy check to get rid of the -Wunused-variable gcc warning
    if (outTexture) {
    }
#else
    // Check for memory leaks and get rid of preexisting texture
    if (outTexture) {
      LOGERR(
          "Warning, Memory leak detected! Trying to load a new "
          "texture before calling delete on the old one");

      freeTexture(outTexture);
    }

    // Create texture from surface pixels
    outTexture = SDL_CreateTextureFromSurface(_renderer, surface);

    if (nullptr == outTexture) {
      LOGERR("Unable to create texture! SDL Error: %s", SDL_GetError());
      return EXIT_FAILURE;
    }

    // Get rid of old loaded surface
    freeSurface(surface);
#endif /* USE_SOFTWARE_RENDERER */

  return EXIT_SUCCESS;
}

int32_t Texture::createEmptySurface(const int32_t width, const int32_t height,
                                    SDL_Surface *&outSurface) {
  if (nullptr != outSurface) {
    LOGERR(
        "Warning, outSurface is not empty. Will not create Empty "
        "Surface. Memory leak prevented.");
    return EXIT_FAILURE;
  }

  /** If the depth is 4 or 8 bits, an empty palette will be allocated
   * for the surface.
   * If the depth is greater than 8 bits, the pixel format is set
   * using the flags '[RGB]/[RGBA] mask'.
   * */
  const uint32_t DEPTH = 32;

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
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

#if !USE_SOFTWARE_RENDERER
int32_t Texture::createEmptyTexture(const int32_t width, const int32_t height,
                                    SDL_Texture *&outTexture) {
  if (nullptr != outTexture) {
    LOGERR(
        "Warning, outTexture is not empty. Will not create Empty "
        "Surface. Memory leak prevented.");
    return EXIT_FAILURE;
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
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
#endif /* !USE_SOFTWARE_RENDERER */

int32_t Texture::clearCurrentRendererTarget(const Color &clearColor) {
#if USE_SOFTWARE_RENDERER
  if (EXIT_SUCCESS != SDL_FillRect(_renderer,  // current target
                                   nullptr,    // nullptr for whole rectangle
                                   clearColor.get32BitRGBA())) {
    LOGERR("Error in SDL_FillRect");
    return EXIT_FAILURE;
  }
#else
  Color currRendererColor = Colors::BLACK;
  bool isSameColorAsOld = true;

  // remember old renderer color
  if (EXIT_SUCCESS != SDL_GetRenderDrawColor(
                          _renderer, &currRendererColor.rgba.r,
                          &currRendererColor.rgba.g, &currRendererColor.rgba.b,
                          &currRendererColor.rgba.a)) {
    LOGERR("Error in, SDL_GetRenderDrawColor(), SDL Error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  if (currRendererColor != clearColor) {
    isSameColorAsOld = false;
  }

  if (isSameColorAsOld) {
    // clear target
    if (EXIT_SUCCESS != SDL_RenderClear(_renderer)) {
      LOGERR("Error in, SDL_RenderClear(), SDL Error: %s", SDL_GetError());
      return EXIT_FAILURE;
    }
  } else  // colour should be changed temporary
  {
    // set new renderer colour
    if (EXIT_SUCCESS != SDL_SetRenderDrawColor(
                            _renderer, clearColor.rgba.r, clearColor.rgba.g,
                            clearColor.rgba.b, clearColor.rgba.a)) {
      LOGERR("Error in, SDL_SetRenderDrawColor(), SDL Error: %s",
             SDL_GetError());
      return EXIT_FAILURE;
    }

    // clear target
    if (EXIT_SUCCESS != SDL_RenderClear(_renderer)) {
      LOGERR("Error in, SDL_RenderClear(), SDL Error: %s", SDL_GetError());
      return EXIT_FAILURE;
    }

    // restore old renderer colour

    if (EXIT_SUCCESS !=
        SDL_SetRenderDrawColor(
            _renderer, currRendererColor.rgba.r, currRendererColor.rgba.g,
            currRendererColor.rgba.b, currRendererColor.rgba.a)) {
      LOGERR("Error in, SDL_SetRenderDrawColor(), SDL Error: %s",
             SDL_GetError());
      return EXIT_FAILURE;
    }
  }
#endif /* USE_SOFTWARE_RENDERER */

  return EXIT_SUCCESS;
}

#if USE_SOFTWARE_RENDERER
int32_t Texture::setRendererTarget(SDL_Surface *target)
#else
int32_t Texture::setRendererTarget(SDL_Texture *target)
#endif /* USE_SOFTWARE_RENDERER */
{
  if (nullptr == _renderer) {
    LOGERR(
        "Error, renderer is still not set for Texture. You are missing"
        " Texture::setRenderer() call "
        "in the program initialization process");
    return EXIT_FAILURE;
  }

#if USE_SOFTWARE_RENDERER
  _renderer = target;
#else
  if (EXIT_SUCCESS != SDL_SetRenderTarget(_renderer, target)) {
    LOGERR(
        "Error, default renderer target could not be set. "
        "SDL_SetRenderTarget() failed, SDL Error: %s",
        SDL_GetError());
    return EXIT_FAILURE;
  }
#endif /* USE_SOFTWARE_RENDERER */

  return EXIT_SUCCESS;
}

#if USE_SOFTWARE_RENDERER
void Texture::draw(SDL_Surface *texture, const DrawParams &drawParams)
#else
void Texture::draw(SDL_Texture *texture, const DrawParams &drawParams)
#endif /* USE_SOFTWARE_RENDERER */
{
  const SDL_Rect *SDLFrameRect =
      reinterpret_cast<const SDL_Rect *>(&drawParams.frameRect);

  SDL_Rect renderQuad = { 0, 0, 0, 0 };

  bool rendererClipped = false;

  if (drawParams.hasCrop)  // has crop and no scaling
  {
    renderQuad = {drawParams.frameCropRect.x, drawParams.frameCropRect.y,
                  drawParams.frameCropRect.w, drawParams.frameCropRect.h};

    if (drawParams.hasScaling)  // has crop and scaling
    {
#if !USE_SOFTWARE_RENDERER
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
        } else {
          rendererClipped = true;
          renderQuad.w = drawParams.scaledWidth;
        }
      }

      if (drawParams.scaledHeight > drawParams.frameCropRect.h) {
        if (!rendererClipped) {
          if (EXIT_SUCCESS != SDL_RenderSetClipRect(_renderer, &renderQuad)) {
            LOGERR("Error in SDL_RenderSetClipRect, SDL Error: %s",
                   SDL_GetError());

            return;
          } else {
            rendererClipped = true;
          }
        }

        renderQuad.h = drawParams.scaledHeight;
      }
#else
      // dummy check to satisfy Wunused-variable gcc warning
      if (rendererClipped) {
      }

      LOGERR(
          "Crop + scaling is not supported for software renderer."
          "Scaling will be ignored. Problem came from widget "
          "with rsrcId: %#16lX",
          drawParams.rsrcId);
#endif /* !USE_SOFTWARE_RENDERER */
    }
  } else if (drawParams.hasScaling)  // has no crop and scaling
  {
    renderQuad = {drawParams.pos.x, drawParams.pos.y, drawParams.scaledWidth,
                  drawParams.scaledHeight};
  } else  // has no crop and no scaling
  {
    renderQuad = {drawParams.pos.x, drawParams.pos.y, drawParams.frameRect.w,
                  drawParams.frameRect.h};
  }

  // Empty renderQuad rectangle -> do not make a render call
  if (renderQuad.w == 0 || renderQuad.h == 0) {
    return;
  }

#if USE_SOFTWARE_RENDERER
  int32_t cameraOffsetX = 0;
  int32_t cameraOffsetY = 0;

  Camera::getCameraOffset(&cameraOffsetX, &cameraOffsetY);

  renderQuad.x -= cameraOffsetX;
  renderQuad.y -= cameraOffsetY;

  // Apply the image
  if (EXIT_SUCCESS != SDL_BlitSurface(texture,       // source surface
                                      SDLFrameRect,  // source rectangle
                                      _renderer,     // destination surface
                                      &renderQuad))  // destination rectangle
  {
    LOGERR(
        "Error in SDL_BlitSurface(), SDL Error: %s from widget with "
        "rsrcId: %#16lX ",
        SDL_GetError(), drawParams.rsrcId);

    return;
  }
#else
  // Render to screen
  if (EXIT_SUCCESS !=
      SDL_RenderCopyEx(_renderer,         // the hardware renderer
                       texture,           // source texture
                       SDLFrameRect,      // source rectangle
                       &renderQuad,       // destination rectangle
                       drawParams.angle,  // rotation angles
                       reinterpret_cast<const SDL_Point *>(
                           drawParams.center),  // rotation center
                       SDL_FLIP_NONE))          // flip mode
  {
    LOGERR(
        "Error in SDL_RenderCopyEx(), SDL Error: %s from widget with "
        "rsrcId: %#16lX ",
        SDL_GetError(), drawParams.rsrcId);

    return;
  }

  // if clipping was done on the Hardware renderer -> reset it to default
  // monitor rectangle
  if (rendererClipped) {
    if (EXIT_SUCCESS !=
        SDL_RenderSetClipRect(
            _renderer, reinterpret_cast<const SDL_Rect *>(_monitorRect))) {
      LOGERR("Error in SDL_RenderSetClipRect(), SDL Error: %s", SDL_GetError());

      return;
    }
  }
#endif /* USE_SOFTWARE_RENDERER */
}

#if USE_SOFTWARE_RENDERER
void Texture::setRenderer(SDL_Surface *renderer)
#else
void Texture::setRenderer(SDL_Renderer *renderer)
#endif /* USE_SOFTWARE_RENDERER */
{
  _renderer = renderer;
}

#if USE_SOFTWARE_RENDERER
void Texture::setAlpha(SDL_Surface *texture, const int32_t alpha) {
  // dummy check to get rid of the -Wunused-variable gcc warning
  if (texture || alpha) {
  }

  LOGERR(
      "Warning, alpha channel (widget transparency) is not supported "
      "for Software renderer. Alpha will not be changed.");
}
#else
void Texture::setAlpha(SDL_Texture *texture, const int32_t alpha) {
  if (EXIT_SUCCESS != SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND)) {
    LOGERR(
        "Warning, SDL_SetTextureBlendMode() failed. Reason: "
        "invalid texture or alpha modulation is not supported. The "
        "closest supported mode will be used. SDL Error: %s",
        SDL_GetError());
  }

  if (EXIT_SUCCESS !=
      SDL_SetTextureAlphaMod(texture, static_cast<uint8_t>(alpha))) {
    LOGERR(
        "Warning, .setAlpha() method will not take effect. Reason: "
        "invalid texture or alpha modulation is not supported. "
        "SDL_SetTextureAlphaMod() failed. SDL Error: %s",
        SDL_GetError());

    return;
  }
}
#endif /* USE_SOFTWARE_RENDERER */

#if USE_SOFTWARE_RENDERER
int32_t Texture::setBlendMode(SDL_Surface *texture, const int32_t blendMode) {
  // dummy check to get rid of the -Wunused-variable gcc warning
  if (texture || blendMode) {
  }

  LOGR(
      "Warning, .setBlendMode() is not supported for Software renderer. "
      "Blend mode will not be changed.");

  return EXIT_SUCCESS;
}
#else
int32_t Texture::setBlendMode(SDL_Texture *texture, const int32_t blendMode) {
  if (EXIT_SUCCESS !=
      SDL_SetTextureBlendMode(texture, static_cast<SDL_BlendMode>(blendMode))) {
    LOGERR(
        "Warning, .setBlendMode() method will not take effect. Reason: "
        "invalid texture or blend mode is not supported. "
        "SDL_SetTextureBlendMode() failed. SDL Error: %s",
        SDL_GetError());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
#endif /* USE_SOFTWARE_RENDERER */
