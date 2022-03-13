#ifndef SDL_UTILS_INPUTEVENTGENERATOR_H_
#define SDL_UTILS_INPUTEVENTGENERATOR_H_

// System headers
#include <cstdint>

// Other libraries headers
#include "utils/ErrorCode.h"

// Own components headers
#include "sdl_utils/input/TouchEntityInterface.h"
#include "sdl_utils/input/InputEvent.h"

// Forward Declarations
class TouchEntity;

// Do not expose SDL_Event internals to the system
union SDL_Event;

struct PollEventOutcome {
  InputEvent event;
  bool hasEvent = true;
};

class InputEventGenerator final : public TouchEntityInterface {
public:
  /** @brief used to allocate memory for the OS dedicated communication
   *        mechanism and initialise SDL support for user defined events
   *
   *  @return ErrorCode - error code
   *  */
  ErrorCode init();

  /** @brief deallocates memory for the OS dedicated communication mechanism
   * */
  void deinit();

  /** @brief used to check if there are any present events on the event queue.
   *
   *  @returns PollEventOutcome - has event(populated) or not
   * */
  PollEventOutcome pollEvent();

  /** @brief used to remember last clicked TouchEntity in order to
   *         perform operations:
   *             > TouchEntity::onLeave();
   *             > TouchEntity::onReturn();
   *         when necessary.
   *
   *  @param TouchEntity * - last clicked TouchEntity
   * */
  void setLastClicked(TouchEntity* lastClicked) override;

private:
  /** @brief used to fill system events from SDL event
   *
   *  @param const uint32_t - SDL event type parsed from the event queue
   * */
  void setEventTypeInternal(InputEvent& e);

  /** @brief used to perform internal motion/drag related checks
   *
   *  @param InputEvent& - populated InputEvent
   * */
  void validateTouchEventMotion(InputEvent& e);

  /** @brief used to perform internal release related checks
   *
   *  @param InputEvent& - populated InputEvent
   * */
  void validateTouchEventRelease(InputEvent& e);

  /** @brief used to pop unused development events out of the event queue
   *
   *  @return bool - is unused development event or not
   * */
  bool isUnusedDevelopmentEvent() const;

  /** Holds the OS communication event system */
  SDL_Event *_sdlEvent = nullptr;

  /** Holds last clicked TouchEntity in order to perform operations:
   *             > TouchEntity::onLeave();
   *             > TouchEntity::onReturn();
   * */
  TouchEntity *_lastClicked = nullptr;

  /** Used in order to perform:
   *             > TouchEntity::onLeave();
   *             > TouchEntity::onReturn();
   * */
  bool _hasLastClickedLeftBoundary = false;
};

#endif /* SDL_UTILS_INPUTEVENTGENERATOR_H_ */
