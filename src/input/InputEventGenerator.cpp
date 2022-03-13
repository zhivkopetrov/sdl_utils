// Corresponding header
#include "sdl_utils/input/InputEventGenerator.h"

// System headers

// Other libraries headers
#include <SDL_events.h>
#include "utils/drawing/Rectangle.h"
#include "utils/Log.h"

// Own components headers
#include "sdl_utils/input/defines/EventDefines.h"
#include "sdl_utils/input/TouchEntity.h"

ErrorCode InputEventGenerator::init() {
  _sdlEvent = new SDL_Event;

  if (nullptr == _sdlEvent) {
    LOGERR("Error, bad alloc for SDL_Event");
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}

void InputEventGenerator::deinit() {
  if (nullptr != _sdlEvent) { //sanity check
    delete _sdlEvent;
    _sdlEvent = nullptr;
  }
}

PollEventOutcome InputEventGenerator::pollEvent() {
  PollEventOutcome outcome{this};

  while (true) {
    //Poll for currently pending events
    if (0 == SDL_PollEvent(_sdlEvent)) {
      outcome.hasEvent = false;
      return outcome; //event queue is empty
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

  //capture mouse position on the screen
  SDL_GetMouseState(&outcome.event.pos.x, &outcome.event.pos.y);
  setEventTypeInternal(outcome.event);
  return outcome; //InputEvent found on the event queue
}

void InputEventGenerator::setLastClicked(TouchEntity *lastClicked) {
  _lastClicked = lastClicked;
}

bool InputEventGenerator::isUnusedDevelopmentEvent() const {
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

void InputEventGenerator::setEventTypeInternal(InputEvent& e) {
  //set type and _systemType events
  switch (_sdlEvent->type) {
  case EventType::KEYBOARD_PRESS:
    e.systemType = SystemEvent::KEYBOARD_PRESS;
    e.type = TouchEvent::KEYBOARD_PRESS;
    e.key =
        _sdlEvent->key.keysym.sym == Keyboard::KEY_ESCAPE ?
            Keyboard::KEY_ESCAPE : Keyboard::KEY_UNKNOWN;
    e.mouseButton = Mouse::UNKNOWN;
    break;

  case EventType::KEYBOARD_RELEASE:
    e.systemType = SystemEvent::KEYBOARD_RELEASE;
    e.type = TouchEvent::KEYBOARD_RELEASE;
    e.key = _sdlEvent->key.keysym.sym;
    e.mouseButton = Mouse::UNKNOWN;
    break;

    //NOTE: the fall-through is intentional
  case EventType::MOUSE_PRESS:
  case EventType::FINGER_PRESS:
    e.systemType = SystemEvent::UNKNOWN;
    e.type = TouchEvent::TOUCH_PRESS;
    e.key = Keyboard::KEY_UNKNOWN;
    e.mouseButton = _sdlEvent->button.button;
    break;

    //NOTE: the fall-through is intentional
  case EventType::MOUSE_MOTION:
  case EventType::FINGER_MOTION:
    validateTouchEventMotion(e);
    break;

    //NOTE: the fall-through is intentional
  case EventType::MOUSE_RELEASE:
  case EventType::FINGER_RELEASE:
  case EventType::MOUSE_WHEEL_MOTION:
    validateTouchEventRelease(e);
    break;

    //X is pressed on the window
  case EventType::QUIT:
    e.systemType = SystemEvent::QUIT;
    e.type = TouchEvent::UNKNOWN;
    e.key = Keyboard::KEY_UNKNOWN;
    e.mouseButton = Mouse::UNKNOWN;
    break;

  default:
    LOGERR("Warning, unknown SDL event type: %u -> " "expand the event list",
        _sdlEvent->type);
    e.systemType = SystemEvent::UNKNOWN;
    e.type = TouchEvent::UNKNOWN;
    e.key = Keyboard::KEY_UNKNOWN;
    e.mouseButton = Mouse::UNKNOWN;
    break;
  }
}

void InputEventGenerator::validateTouchEventMotion(InputEvent& e) {
  /** If _hasLastClickedLeftBoundary is true this means a PRESS was made
   *  inside the touch entity and it was DRAGGED outside the
   *  touchEntityEventRect, which resulted in TouchEntity::onLeave() call.
   * */
  if (_hasLastClickedLeftBoundary) {
    /** The user is still holding the PRESS and returns to the
     *  touchEntityEventRect -> this means TouchEntity::onReturn()
     *  should be invoked and the event type should be set to DRAG.
     * */
    if (Rectangle::isPointInRect(e.pos, *_lastClicked->touchEntityEventRect)) {
      _lastClicked->onReturn(e);
      _hasLastClickedLeftBoundary = false;

      e.type = TouchEvent::TOUCH_DRAG;
      e.systemType = SystemEvent::UNKNOWN;
    }
    /** The user is still holding the PRESS but still has not returned
     *  to the touchEntityEventRect -> this means the event type
     *  should be set to MOTION.
     * */
    else {
      e.type = TouchEvent::UNKNOWN;
      e.systemType = SystemEvent::MOTION;
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
    if (Rectangle::isPointInRect(e.pos, *_lastClicked->touchEntityEventRect)) {
      e.type = TouchEvent::TOUCH_DRAG;
      e.systemType = SystemEvent::UNKNOWN;
    }
    /** The user is still holding the PRESS and has exited outside of
     *  the touchEntityEventRect -> this means TouchEntity::onLeave()
     *  should be invoked and the event type should be set to MOTION.
     * */
    else {
      _lastClicked->onLeave(e);
      _hasLastClickedLeftBoundary = true;

      e.type = TouchEvent::UNKNOWN;
      e.systemType = SystemEvent::MOTION;
    }
  }

  e.key = Keyboard::KEY_UNKNOWN;
  e.mouseButton = _sdlEvent->button.button;
}

void InputEventGenerator::validateTouchEventRelease(InputEvent& e) {
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
      && Rectangle::isPointInRect(e.pos, *_lastClicked->touchEntityEventRect))
      || _sdlEvent->type == EventType::MOUSE_WHEEL_MOTION) {
    e.type = TouchEvent::TOUCH_RELEASE;
    e.systemType = SystemEvent::UNKNOWN;
  } else {
    e.type = TouchEvent::UNKNOWN;
    e.systemType = SystemEvent::EMPTY_TOUCH_RELEASE;
  }

  //reset PRESS holders
  _lastClicked = nullptr;
  _hasLastClickedLeftBoundary = false;

  e.key = Keyboard::KEY_UNKNOWN;
  e.mouseButton = _sdlEvent->button.button;
}
