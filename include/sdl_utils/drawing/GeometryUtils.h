#ifndef SDL_UTILS_GEOMETRYUTILS_H_
#define SDL_UTILS_GEOMETRYUTILS_H_

// System headers

// Other libraries headers
#include "utils/drawing/Rectangle.h"

// Own components headers

// Forward declarations

class GeometryUtils {
public:
  GeometryUtils() = delete;

  static bool findRectIntersection(const Rectangle &srcA,
                                   const Rectangle &srcB,
                                   Rectangle &intersectionResult);
};

#endif /* SDL_UTILS_GEOMETRYUTILS_H_ */
