#ifndef SDL_UTILS_GEOMETRYUTILS_H_
#define SDL_UTILS_GEOMETRYUTILS_H_

// C system headers

// C++ system headers

// Other libraries headers

// Own components headers
#include "utils/drawing/Rectangle.h"

// Forward declarations

class GeometryUtils {
public:
  GeometryUtils() = delete;

  bool findRectIntersection(const Rectangle &srcA,
                            const Rectangle &srcB,
                            Rectangle &intersectionResult);
};

#endif /* SDL_UTILS_GEOMETRYUTILS_H_ */
