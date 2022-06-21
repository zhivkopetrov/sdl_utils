#ifndef SDL_UTILS_TEXTURE_H_
#define SDL_UTILS_TEXTURE_H_

// System headers
#include <cstdint>

// Other libraries headers

// Own components headers
#include "sdl_utils/drawing/defines/RendererDefines.h"
#include "sdl_utils/drawing/defines/DrawConstants.h"
#include "utils/ErrorCode.h"

// Forward declarations
class Color;
struct Rectangle;
struct SDL_Rect;
struct SDL_Point;
struct SDL_Surface;
struct SDL_Texture;
struct SDL_Renderer;
struct DrawParams;
typedef struct _TTF_Font TTF_Font;

class Texture {
public:
  // forbid the default constructor and destructor
  Texture() = delete;

  /** @brief used to free SDL_Surface
   *
   *  @param SDL_Surface *& the surface to be freed
   * */
  static void freeSurface(SDL_Surface *&surface);

  /** @brief used to free SDL_Texture
   *
   *  @param SDL_Texture*& the texture to be freed
   * */
  static void freeTexture(SDL_Texture *&texture);

  /** @brief used to set monitor rectangle -> so when renderer clipping
   *         is performed, the clip could be reset back to
   *                                  normal(monitor rectangle) boundary
   *
   *  @param const Rectangle & - the monitor window rectangle
   *  */
  static void setMonitorRect(const Rectangle &monitorRect);

  /** @brief used acquire the TTF text dimension without the need of the
   *         text to actually be renderer.
   *
   *         NOTE: even thought text rendering is not done, this
   *               method is again pretty complex -> so don't invoke it
   *               for no reason.
   *
   *  @param const char * - user provided text
   *  @param TTF_Font *   - SDL_Font that is used
   *  @param int32_t &    - width of the text if it would have been rendered
   *  @param int32_t &    - height of the if it would have been rendered
   *
   *  @returns ErrorCode  - error code
   * */
  static ErrorCode getTextDimensions(const char *text, TTF_Font *font,
                                     int32_t &outTextWidth,
                                     int32_t &outTextHeight);

  /** @brief used to load SDL_Surface from file on the hard drive
   *
   *  @param const char *   - absolute path to file
   *  @param SDL_Surface *& - dynamically created SDL_Surface
   *
   *  @returns ErrorCode    - error code
   * */
  static ErrorCode loadSurfaceFromFile(const char *path,
                                       SDL_Surface *&outSurface);

  /** @brief used to create SDL_Texture from provided SDL_Surface
   *         NOTE: if SDL_Texture is successful - the input SDL_Surface
   *                       is not longer needed -> therefore it is freed.
   *
   *  @param SDL_Surface *& - input SDL_Surface
   *  @param SDL_Texture *& - dynamically created SDL_Texture
   *
   *  @returns int32_t      - error code
   * */
  static ErrorCode loadTextureFromSurface(SDL_Surface *&surface,
                                          SDL_Texture *&outTexture);

  /** @brief used to create a 32-bit surface with the bytes of each pixel
   *         in R,G,B,A order, as expected by OpenGL for textures
   *
   *  @param const int32_t  - surface width
   *  @param const int32_t  - surface height
   *  @param SDL_Surface *& - pointer to newly created SDL_Surface
   *                                                 (nullptr on failure)
   *
   *         WARNING: memory for outSurface is dynamically allocated by
   *                  SDL. It's up to the developer to call
   *                  SDL_FreeSurface() on the created Surface when he is
   *                  done dealing with the memory. Otherwise memory leak
   *                  will occur.
   *
   *  @return ErrorCode     - error code
   * */
  static ErrorCode createEmptySurface(const int32_t width, const int32_t height,
                                      SDL_Surface *&outSurface);

  /** @brief used to clear (wipe out) current renderer target with
   *         currently set draw color for the main renderer
   *
   *         WARNING: use this method only if you know what you are doing!
   *
   *  @param const Color & - the clear color for the hardware renderer
   *
   *  @return ErrorCode - error code
   * */
  static ErrorCode clearCurrentRendererTarget(const Color &clearColor);

  /** @brief used to change the target for main graphical renderer.
   *
   *         WARNING: use this method only if you know what you are doing!
   *
   *  @param SDL_Texture *  - the new target for the hardware renderer
   *                          (use nullptr as target param in order
   *                                 to reset to default renderer target)
   *
   *  @return ErrorCode     - error code
   * */
  static ErrorCode setRendererTarget(SDL_Texture *target);

  /** @brief used to load SDL_Texture from provided user text
   *
   *  @param const char *   - user provided text
   *  @param TTF_Font *     - SDL_Font that is used
   *  @param const Color &  - color used to create the text
   *  @param SDL_Texture *& - dynamically created SDL_Surface
   *  @param int32_t &      - width of the created SDL_Surface text
   *  @param int32_t &      - height of the created SDL_Surface text
   *
   *  @returns ErrorCode    - error code
   * */
  static ErrorCode loadFromText(const char *text, TTF_Font *font,
                                const Color &color, SDL_Texture *&outTexture,
                                int32_t &outTextWidth, int32_t &outTextHeight);

  /** @brief used to render the input SDL_Texture widget with it's
   *                                       corresponding draw parameters.
   *
   *  @param SDL_Texture *      - texture to be drawn
   *  @param const DrawParams & - draw parameters
   * */
  static void draw(SDL_Texture *texture, const DrawParams &drawParams);

  /** @brief used to acquire renderer pointer that will be performing
   *                                         the graphical render calls.
   *
   *  @param SDL_Renderer * - the actual hardware renderer
   * */
  static void setRenderer(SDL_Renderer *renderer);

  /** @brief used to change the alpha channel (Widget transparency)
   *
   *         NOTE: alpha channel is only supported by Hardware renderer
   *
   *  @param SDL_Texture * - texture to be modified
   *  @param const int32_t - new alpha channel value
   * */
  static void setAlpha(SDL_Texture *texture, const int32_t alpha);

  /** @brief used to change the Widget blend mode (used for calculations
   *         on the SDL_Texture RGBA pixels)
   *
   *         NOTE: blend mode is only supported by Hardware renderer
   *
   *  @param SDL_Texture *   - texture to be modified
   *  @param const BlendMode - new blend mode value
   *
   *  @return ErrorCode      - error code
   * */
  static ErrorCode setBlendMode(SDL_Texture *texture,
                                const BlendMode blendMode);

  /** @brief to create a texture for the current rendering context.
   *
   *  @param const int32_t  - Texture width
   *  @param const int32_t  - Texture height
   *  @param SDL_Texture *& - pointer to newly created SDL_Texture
   *                                                 (nullptr on failure)
   *
   *         WARNING: memory for outTexture is dynamically allocated by
   *                  SDL. It's up to the developer to call
   *                  SDL_FreeTexture() on the created Texture when he is
   *                  done dealing with the memory. Otherwise memory leak
   *                  will occur.
   *
   *  @return ErrorCode     - error code
   * */
  static ErrorCode createEmptyTexture(const int32_t width, const int32_t height,
                                      SDL_Texture *&outTexture);

  /** @brief takes a snapshot of current renderer pixels
   *
   *  @param const char*               - file path
   *  @param const ScreenshotContainer - type of container [PNG, JPG, ...]
   *  @param const int32_t             - quality (applied only for JPG)
   *                                     range: [0, 100], worst(0) - best(100)
   *
   *  WARNING: this method is quite slow and should not be used in performance
   *           critical parts of the code
   *
   *  @return ErrorCode                - error code
   * */
  static ErrorCode takeScreenshot(const char *file,
                                  const ScreenshotContainer container,
                                  const int32_t quality = 10);

private:
  /** Keep pointer to the actual renderer,
   * since Texture class function will be used all the time
   * and in order not to pass pointers on every single function call
   * */
  static SDL_Renderer *_renderer;

  static Rectangle _monitorRect;
};

#endif /* SDL_UTILS_TEXTURE_H_ */
