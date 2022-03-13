#ifndef SDL_UTILS_TOUCHENTITYINTERFACE_H_
#define SDL_UTILS_TOUCHENTITYINTERFACE_H_

// System headers

// Other libraries headers

// Own components headers

// Forward Declarations
class TouchEntity;

class TouchEntityInterface {
public:
  virtual ~TouchEntityInterface() noexcept = default;

  virtual void setLastClicked(TouchEntity* lastClicked) = 0;
};

#endif /* SDL_UTILS_TOUCHENTITYINTERFACE_H_ */
