// Corresponding header
#include "sdl_utils/input/InputEvent.h"

// System headers

// Other libraries headers

// Own components headers
#include "sdl_utils/input/TouchEntityInterface.h"

InputEvent::InputEvent(TouchEntityInterface *touchEntityInterface)
    : key(Keyboard::KEY_UNKNOWN), mouseButton(Mouse::UNKNOWN),
      type(TouchEvent::UNKNOWN), systemType(SystemEvent::UNKNOWN),
      _touchEntityInterface(touchEntityInterface) {

}

bool InputEvent::checkForExitRequest() const {
  return SystemEvent::QUIT == systemType || Keyboard::KEY_ESCAPE == key;
}

void InputEvent::setLastClicked(TouchEntity *lastClicked) const {
  _touchEntityInterface->setLastClicked(lastClicked);
}
