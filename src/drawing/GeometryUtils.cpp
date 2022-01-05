// Corresponding header
#include "sdl_utils/drawing/GeometryUtils.h"

// C system headers

// C++ system headers

// Other libraries headers
#include <SDL_rect.h>

// Own components headers

bool GeometryUtils::findRectIntersection(const Rectangle &srcA,
                                         const Rectangle &srcB,
                                         Rectangle &intersectionResult) {
  return SDL_IntersectRect(reinterpret_cast<const SDL_Rect *>(&srcA),
                           reinterpret_cast<const SDL_Rect *>(&srcB),
                           reinterpret_cast<SDL_Rect *>(&intersectionResult));
}

