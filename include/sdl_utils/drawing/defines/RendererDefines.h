#ifndef SDL_UTILS_RENDERERDEFINES_H_
#define SDL_UTILS_RENDERERDEFINES_H_

// System headers
#include <cstdint>

// Other libraries headers

// Own components headers

/* Values:
 *
 * USE_ANTI_ALIASING_ON_TEXT 1
 * Text quality is better, because Anti-Aliasing is applied. What this means is
 * that alpha blending to dither the font with the given color is applied.
 * This created a way smoother text, but is around 20% - 40% slower that the
 * fast low quality version.
 *
 *
 * USE_ANTI_ALIASING_ON_TEXT 0
 * The quality is poor, because the text is pixel drawn resulting in a "edge"
 * shaped text. While the text is not so good looking it is way more faster
 * than it's Anti-Aliasing counterpart.
 * */
#define USE_ANTI_ALIASING_ON_TEXT 1

/* Values:
 *
 * DISABLE_DOUBLE_BUFFERING_SWAP_INTERVAL 0
 * Does not interfere with swap interval. This will result in underlying
 * graphics API "swap_buffers" command to block and assure the frame capping
 * set by the monitor max refresh rate.
 *
 * DISABLE_DOUBLE_BUFFERING_SWAP_INTERVAL 1
 * Make the double buffering swap interval to 0. What this means is that the
 * underlying graphics API, will not block "swap_buffers" command and will
 * leave control /frame capping/ to the application side.
 *
 * IMPORTANT NOTE: this feature will work only when VSync is enabled.
 * */
#define DISABLE_DOUBLE_BUFFERING_SWAP_INTERVAL 0

/* Values:
 * ENABLE_VSYNC 1
 * Enabled VSync, which caps the application frame rate to the maximum monitor
 * refresh rate.
 *
 * ENABLE_VSYNC 0
 * Disables VSync, which leaves the frame rate control to be handled by the
 * application.
 * */
#define ENABLE_VSYNC 0

enum class RendererCmd : uint8_t {
  CLEAR_SCREEN = 0,
  FINISH_FRAME,
  CHANGE_CLEAR_COLOR,
  LOAD_TEXTURE_SINGLE,
  LOAD_TEXTURE_MULTIPLE,
  DESTROY_TEXTURE,
  CREATE_FBO,   // FBO (frame buffer object)
  DESTROY_FBO,  // FBO (frame buffer object)
  CHANGE_RENDERER_TARGET,
  RESET_RENDERER_TARGET,
  CLEAR_RENDERER_TARGET,
  UPDATE_RENDERER_TARGET,
  CHANGE_TEXTURE_BLENDMODE,
  CHANGE_TEXTURE_OPACITY,
  CREATE_TTF_TEXT,
  RELOAD_TTF_TEXT,
  DESTROY_TTF_TEXT,
  ENABLE_DISABLE_MULTITHREAD_TEXTURE_LOADING,
  TAKE_SCREENSHOT,
  EXIT_RENDERING_LOOP,

  UNDEFINED = 255
};

#endif /* SDL_UTILS_RENDERERDEFINES_H_ */
