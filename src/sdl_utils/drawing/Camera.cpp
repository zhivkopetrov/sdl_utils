// Corresponding header
#include "sdl_utils/drawing/Camera.h"

// C system headers

// C++ system headers

// Other libraries headers

// Own components headers
#include "utils/Log.h"

#if USE_SOFTWARE_RENDERER

/** Warning, do not initialize _cameraRect as a Rectangle for 2 reasons:
 *      > we could try to initialize the static variable as
 *          Rectangle Camera::_cameraRect = Rectangle::UNDEFINED, but since
 *          they are both static(global) object their order or creating is
 *          unknown -> and that leads to undefined behavior
 *
 *      > we could try to initialize the static variable as
 *          Rectangle Camera::_cameraRect, but when game shared object is opened
 *          this will invoke the default constructor for Rectangle and override
 *          the values set by setCameraDimensions()
 *
 *  Conclusion: use 4 different variables.
 */

int32_t Camera::_cameraRectX = 0;
int32_t Camera::_cameraRectY = 0;
int32_t Camera::_cameraRectW = 0;
int32_t Camera::_cameraRectH = 0;
int32_t Camera::_screenWidth = 0;
int32_t Camera::_screenHeight = 0;

void Camera::setCameraDimensions(const int32_t cameraWidth,
                                 const int32_t cameraHeight,
                                 const int32_t screenWidth,
                                 const int32_t screenHeight) {
  _cameraRectX = 0;
  _cameraRectY = 0;
  _cameraRectW = cameraWidth;
  _cameraRectH = cameraHeight;

  _screenWidth = screenWidth;
  _screenHeight = screenHeight;
}

void Camera::moveCameraX(const int32_t x) {
  // only checking for one of the 4 variables from the
  // init function (setCameraDimensions) should be enough
  if (_screenWidth == 0) {
    LOGERR("Camera dimensions are not set! Will not move camera.");
    return;
  }

  if (0 == x) {
    return;  // no update needed
  }

  _cameraRectX += x;

  // X axis evaluation
  if (0 < x)  // positive X offset -> border by screen right X coordinate
  {
    const int32_t CORRECTION_X = (_cameraRectX + _cameraRectW) - _screenWidth;

    if (0 < CORRECTION_X) {
      _cameraRectX -= CORRECTION_X;
    }
  } else  // negative X offset -> border by screen left X coordinate
  {
    if (0 > _cameraRectX) {
      _cameraRectX = 0;
    }
  }
}

void Camera::moveCameraY(const int32_t y) {
  // only checking for one of the 4 variables from the
  // init function (setCameraDimensions) should be enough
  if (_screenWidth == 0) {
    LOGERR("Camera dimensions are not set! Will not move camera.");
    return;
  }

  if (0 == y) {
    return;  // no update needed
  }

  _cameraRectY += y;

  // Y axis evaluation
  if (0 > y)  // negative Y offset -> border by screen upper Y coordinate
  {
    if (0 > _cameraRectY) {
      _cameraRectY = 0;
    }
  } else  // positive Y offset -> border by screen lower Y coordinate
  {
    const int32_t CORRECTION_Y = (_cameraRectY + _cameraRectH) - _screenHeight;

    if (0 < CORRECTION_Y) {
      _cameraRectY -= CORRECTION_Y;
    }
  }
}

void Camera::getCameraOffset(int32_t* x, int32_t* y) {
  // only checking for one of the 4 variables from the
  // init function (setCameraDimensions) should be enough
  if (_screenWidth == 0) {
    LOGERR("Camera dimensions are not set! Returning zero values.");
    *x = 0;
    *y = 0;
    return;
  }

  *x = _cameraRectX;
  *y = _cameraRectY;
}

void Camera::moveCamera(const int32_t x, const int32_t y) {
  moveCameraX(x);
  moveCameraY(y);
}

#endif /* USE_SOFTWARE_RENDERER */
