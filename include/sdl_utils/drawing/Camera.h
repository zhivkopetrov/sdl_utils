/*
 * Camera.h
 *
 *  @brief a class used to move camera capture.
 *      Motivation: Since camera utility is used with combination with
 *      SOFTWARE_RENDERER and the latter requires a small window to be able
 *      to operate -> the user is not able to see all of the GUI.
 *
 *      Now this can be achieved by using the camera.
 *
 *      USAGE: Move the camera with the keyboard arrows <> /\ \/
 *
 *      NOTE: The camera also modifies the event position.
 */

#ifndef SDL_UTILS_CAMERA_H_
#define SDL_UTILS_CAMERA_H_

// C system headers

// C++ system headers
#include <cstdint>

// Other libraries headers
#include "sdl_utils/drawing/RendererDefines.h"

// Own components headers

#if USE_SOFTWARE_RENDERER
class Camera {
 public:
  // forbid the default constructor and destructor
  Camera() = delete;

  /** @brief used to set initial camera dimensions
   *
   *  @param const int32_t - the width of the camera
   *  @param const int32_t - the height of the camera
   *  @param const int32_t - the width of the screen
   *  @param const int32_t - the height of the screen
   * */
  static void setCameraDimensions(const int32_t cameraWidth,
                                  const int32_t cameraHeight,
                                  const int32_t screenWidth,
                                  const int32_t screenHeight);

  /** @brief used to move camera with relative movement to the
   *                      top left corner (0, 0) of the window context
   *
   *  @param const int32_t - relative X offset of the camera
   *  @param const int32_t - relative Y offset of the camera
   * */
  static void moveCamera(const int32_t x, const int32_t y);

  /** @brief used to move camera along the X axis with relative movement
   *  to the left border of the window context
   *
   *  @param const int32_t - relative X offset of the camera
   * */
  static void moveCameraX(const int32_t x);

  /** @brief used to move camera along the Y axis with relative movement
   *  to the top border of the window context
   *
   *  @param const int32_t - relative Y offset of the camera
   * */
  static void moveCameraY(const int32_t y);

  /** @brief used acquire current camera X and Y offset from
   *                      top left corner (0, 0) of the window context
   *
   *  @param int32_t * - out X offset of the camera
   *  @param int32_t * - out Y offset of the camera
   * */
  static void getCameraOffset(int32_t* x, int32_t* y);

 private:
  /** Used to determine the camera dimensions
   *  Camera window is defined by
   *  Rectangle(_cameraRectX, _cameraRectY, _screenWidth, _screenHeight)
   */
  static int32_t _cameraRectX;
  static int32_t _cameraRectY;
  static int32_t _cameraRectW;
  static int32_t _cameraRectH;

  /** Used to determine the screen dimensions
   *  Screen window is locked between the
   *  Rectangle(0, 0, _screenWidth, _screenHeight)
   */
  static int32_t _screenWidth;
  static int32_t _screenHeight;
};

#endif /* USE_SOFTWARE_RENDERER */

#endif /* SDL_UTILS_CAMERA_H_ */
