#ifndef SDL_UTILS_INPUTEVENT_H_
#define SDL_UTILS_INPUTEVENT_H_

// C system headers

// C++ system headers
#include <cstdint>

// Other libraries headers
#include "utils/drawing/Point.h"

// Own components headers
#include "sdl_utils/input/defines/EventDefines.h"

// Forward Declarations
class TouchEntity;

// Do not expose SDL_Event internals to the system
union SDL_Event;

class InputEvent {
public:
  InputEvent();

  /** @brief used to allocate memory for the OS dedicated communication
   *        mechanism and initialise SDL support for user defined events
   *
   *  @return int32_t - error code
   *  */
  int32_t init();

  /** @brief deallocates memory for the OS dedicated
   *                                              communication mechanism
   * */
  void deinit();

  /** @brief used to check if there are any present events
   *                                                   on the event queue.
   *         - if yes - "type" and "key" member variables are set
   *                         and current event is popped out of the queue;
   *         - if no  - nothing is done;
   *
   *  @returns bool - has event or not
   * */
  bool pollEvent();

  /** @brief used to check user has requested system close by either:
   *           > Pressing the ESCAPE key;
   *           > Pressing the X graphical key on the window;
   *           > Sending SIGINT signal to the application (CTRL + C) keys;
   *
   *         NOTE: All of the above cases could only happen during
   *                                                          DEVELOPMENT.
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
  void setLastClicked(TouchEntity *lastClicked) const {
    _lastClicked = lastClicked;
  }

  /* Holds event position on the screen */
  Point pos;

  /* See EventDefines.h for more information */
  int32_t key;
  uint8_t mouseButton;
  TouchEvent type;

private:
  /** @brief used to fill system events from SDL event
   *
   *  @param const uint32_t - SDL event type parsed from the event queue
   * */
  void setEventTypeInternal();

  /** @brief used to perform internal motion/drag related checks
   *
   *         NOTE: event type and event systemType may be modified
   *                                    internally inside the method body
   * */
  void validateTouchEventMotion();

  /** @brief used to perform internal release related checks
   *
   *         NOTE: event type and event systemType may be modified
   *                                    internally inside the method body
   * */
  void validateTouchEventRelease();

  /** @brief used to pop unused development events out of the event queue
   *
   *  @return bool - is unused development event or not
   * */
  bool isUnusedDevelopmentEvent();

  /** Holds the OS communication event system */
  SDL_Event *_sdlEvent;

  /** Holds last clicked TouchEntity in order to perform operations:
   *             > TouchEntity::onLeave();
   *             > TouchEntity::onReturn();
   * */
  mutable TouchEntity *_lastClicked;

  /* See EventDefines.h for more information */
  SystemEvent _systemType;

  /** Used in order to perform:
   *             > TouchEntity::onLeave();
   *             > TouchEntity::onReturn();
   * */
  mutable bool _hasLastClickedLeftBoundary;
};

#endif /* SDL_UTILS_INPUTEVENT_H_ */
