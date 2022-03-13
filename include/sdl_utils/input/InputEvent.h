#ifndef SDL_UTILS_INPUTEVENT_H_
#define SDL_UTILS_INPUTEVENT_H_

// System headers
#include <cstdint>

// Other libraries headers
#include "utils/drawing/Point.h"

// Own components headers
#include "sdl_utils/input/defines/EventDefines.h"

// Forward Declarations
class TouchEntityInterface;
class TouchEntity;

class InputEvent {
public:
  InputEvent(TouchEntityInterface* touchEntityInterface);

  /** @brief used to check user has requested system close by either:
   *           > Pressing the ESCAPE key;
   *           > Pressing the X graphical key on the window;
   *           > Sending SIGINT signal to the application (CTRL + C) keys;
   *
   *  @returns bool - should exit application or not
   * */
  bool checkForExitRequest() const;

  /** @brief used to remember last clicked TouchEntity in order to
   *         perform operations:
   *             > TouchEntity::onLeave();
   *             > TouchEntity::onReturn();
   *         when necessary.
   *
   *  @param TouchEntity * - last clicked TouchEntity
   * */
  void setLastClicked(TouchEntity* lastClicked) const;

  /* Holds event position on the screen */
  Point pos;

  /* See EventDefines.h for more information */
  int32_t key;
  uint8_t mouseButton;
  TouchEvent type;
  SystemEvent systemType;

private:
  TouchEntityInterface* _touchEntityInterface;
};

#endif /* SDL_UTILS_INPUTEVENT_H_ */
