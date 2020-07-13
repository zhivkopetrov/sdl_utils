// Corresponding header
#include "sdl_utils/input/InputEvent.h"

// C system headers

// C++ system headers

// Other libraries headers
#include <SDL_events.h>

// Own components headers
#include "sdl_utils/input/MouseUtils.h"
#include "sdl_utils/input/TouchEntity.h"
#include "sdl_utils/drawing/defines/RendererDefines.h"

#if USE_SOFTWARE_RENDERER
#include "sdl_utils/drawing/Camera.h"
#endif /* USE_SOFTWARE_RENDERER */

#include "utils/Log.h"
#include "utils/data_type/EnumClassUtils.hpp"
#include "utils/drawing/Rectangle.h"

InputEvent::InputEvent()
    : pos(Point::UNDEFINED), key(Keyboard::KEY_UNKNOWN),
      mouseButton(Mouse::UNKNOWN), type(TouchEvent::UNKNOWN),
      _sdlEvent(nullptr), _lastClicked(nullptr),
      _systemType(SystemEvent::UNKNOWN), _hasLastClickedLeftBoundary(false) {

}

int32_t InputEvent::init() {
  int32_t err = EXIT_SUCCESS;

  if (EXIT_SUCCESS == err) {
    _sdlEvent = new SDL_Event;

    if (nullptr == _sdlEvent) {
      LOGERR("Error, bad alloc for SDL_Event");

      err = EXIT_FAILURE;
    }
  }

  return err;
}

void InputEvent::deinit() {
  if (nullptr != _sdlEvent) //sanity check
  {
    delete _sdlEvent;
    _sdlEvent = nullptr;
  }
}

bool InputEvent::pollEvent() {
  while (true) {
    //Poll for currently pending events
    if (0 == SDL_PollEvent(_sdlEvent)) {
      return false; //event queue is empty
    } else {
      /** Skip unused event, because they are flooding the event queue
       *
       *  NOTE: MOUSE_MOTION should only be ignored if there is
       *        no set _lastClicked TouchEntity.
       *  Reason: If we do have _lastClicked TouchEntity set -> this will
       *          result in DRAG event;
       * */
      if (false == isUnusedDevelopmentEvent()) {
        break; //event is normal -> break the endless loop
      }
    }
  }

  /* capture mouse position on the screen */
  SDL_GetMouseState(&pos.x, &pos.y);

  setEventTypeInternal();

  return true; //InputEvent found on the event queue
}

bool InputEvent::isUnusedDevelopmentEvent() {
  bool isUnusedDevEvent = true;

  switch (_sdlEvent->type) {
  case EventType::WINDOW:
  case EventType::TEXT_INPUT:
  case EventType::AUDIO_DEVICE_ADDED:
  case EventType::OS_TEXTEDITING:
  case EventType::OS_LANGUAGE_CHANGED:
    break;

  case EventType::MOUSE_MOTION:
    /** If there is _lastClicked TouchEntity set -> event should
     * be of type DRAG
     * */
    if (nullptr != _lastClicked) {
      isUnusedDevEvent = false;
    }
    break;

  default:
    isUnusedDevEvent = false;
  }

  return isUnusedDevEvent;
}

void InputEvent::setEventTypeInternal() {
  //set type and _systemType events
  switch (_sdlEvent->type) {
  case EventType::KEYBOARD_PRESS:
    _systemType = SystemEvent::KEYBOARD_PRESS;
    type = TouchEvent::KEYBOARD_PRESS;
    key =
        _sdlEvent->key.keysym.sym == Keyboard::KEY_ESCAPE ?
            Keyboard::KEY_ESCAPE : Keyboard::KEY_UNKNOWN;
    mouseButton = Mouse::UNKNOWN;
    break;

  case EventType::KEYBOARD_RELEASE:
    _systemType = SystemEvent::KEYBOARD_RELEASE;
    type = TouchEvent::KEYBOARD_RELEASE;
    key = _sdlEvent->key.keysym.sym;
    mouseButton = Mouse::UNKNOWN;
    break;

    //NOTE: the fall-through is intentional
  case EventType::MOUSE_PRESS:
  case EventType::FINGER_PRESS:
    _systemType = SystemEvent::UNKNOWN;
    type = TouchEvent::TOUCH_PRESS;
    key = Keyboard::KEY_UNKNOWN;
    mouseButton = _sdlEvent->button.button;
    break;

    //NOTE: the fall-through is intentional
  case EventType::MOUSE_MOTION:
  case EventType::FINGER_MOTION:
    validateTouchEventMotion();
    break;

    //NOTE: the fall-through is intentional
  case EventType::MOUSE_RELEASE:
  case EventType::FINGER_RELEASE:
  case EventType::MOUSE_WHEEL_MOTION:
    validateTouchEventRelease();
    break;

    //X is pressed on the window
  case EventType::QUIT:
    _systemType = SystemEvent::QUIT;
    type = TouchEvent::UNKNOWN;
    key = Keyboard::KEY_UNKNOWN;
    mouseButton = Mouse::UNKNOWN;
    break;

  default:
    LOGERR("Warning, unknown SDL event type: %u -> " "expand the event list",
        _sdlEvent->type)
    ;
    _systemType = SystemEvent::UNKNOWN;
    type = TouchEvent::UNKNOWN;
    key = Keyboard::KEY_UNKNOWN;
    mouseButton = Mouse::UNKNOWN;
    break;
  }
}

void InputEvent::validateTouchEventMotion() {
  /** If _hasLastClickedLeftBoundary is true this means a PRESS was made
   *  inside the touch entity and it was DRAGGED outside the
   *  touchEntityEventRect, which resulted in TouchEntity::onLeave() call.
   * */
  if (_hasLastClickedLeftBoundary) {
    /** The user is still holding the PRESS and returns to the
     *  touchEntityEventRect -> this means TouchEntity::onReturn()
     *  should be invoked and the event type should be set to DRAG.
     * */
    if (Rectangle::isPointInRect(pos, *_lastClicked->touchEntityEventRect)) {
      _lastClicked->onReturn(*this);
      _hasLastClickedLeftBoundary = false;

      type = TouchEvent::TOUCH_DRAG;
      _systemType = SystemEvent::UNKNOWN;
    }
    /** The user is still holding the PRESS but still has not returned
     *  to the touchEntityEventRect -> this means the event type
     *  should be set to MOTION.
     * */
    else {
      type = TouchEvent::UNKNOWN;
      _systemType = SystemEvent::MOTION;
    }
  }
  /** If _hasLastClickedLeftBoundary is false this means a PRESS was made
   *  inside the touch entity and it may be.or may not be DRAGGED inside
   *  the touchEntityEventRect.
   * */
  else //_hasLastClickedLeftBoundary == false
  {
    /** The user is still holding the PRESS but is still inside of
     *  the touchEntityEventRect -> this means the event type
     *  should be set to DRAG.
     * */
    if (Rectangle::isPointInRect(pos, *_lastClicked->touchEntityEventRect)) {
      type = TouchEvent::TOUCH_DRAG;
      _systemType = SystemEvent::UNKNOWN;
    }
    /** The user is still holding the PRESS and has exited outside of
     *  the touchEntityEventRect -> this means TouchEntity::onLeave()
     *  should be invoked and the event type should be set to MOTION.
     * */
    else {
      _lastClicked->onLeave(*this);
      _hasLastClickedLeftBoundary = true;

      type = TouchEvent::UNKNOWN;
      _systemType = SystemEvent::MOTION;
    }
  }

  key = Keyboard::KEY_UNKNOWN;
  mouseButton = _sdlEvent->button.button;
}

void InputEvent::validateTouchEventRelease() {
  /** If there is no _lastClicked activated do not result in
   *  TouchEvent::RELEASE, because PRESS did not come from inside any
   *  TouchEntity -> therefore no action should be taken.
   *
   *  On the other hand there could be an active _lastClicked TouchEntity,
   *  but the release could happen outside the
   *  TouchEntity::touchEntityEventRect -> therefore no action
   *  should be taken.
   * */
  if ( ( (nullptr != _lastClicked)
      && Rectangle::isPointInRect(pos, *_lastClicked->touchEntityEventRect))
      || _sdlEvent->type == EventType::MOUSE_WHEEL_MOTION) {
    type = TouchEvent::TOUCH_RELEASE;
    _systemType = SystemEvent::UNKNOWN;
  } else {
    type = TouchEvent::UNKNOWN;
    _systemType = SystemEvent::EMPTY_TOUCH_RELEASE;
  }

  //reset PRESS holders
  _lastClicked = nullptr;
  _hasLastClickedLeftBoundary = false;

  key = Keyboard::KEY_UNKNOWN;
  mouseButton = _sdlEvent->button.button;
}

bool InputEvent::checkForExitRequest() const {
  return SystemEvent::QUIT == _systemType || Keyboard::KEY_ESCAPE == key;
}
