// Corresponding header
#include "sdl_utils/input/InputEvent.h"

// C system headers

// C++ system headers

// Other libraries headers

// Own components headers
#include "sdl_utils/input/TouchEntityInterface.h"

InputEvent::InputEvent(TouchEntityInterface *touchEntityInterface)
    : _touchEntityInterface(touchEntityInterface) {

}

bool InputEvent::checkForExitRequest() const {
  return SystemEvent::QUIT == systemType || Keyboard::KEY_ESCAPE == key;
}

void InputEvent::setLastClicked(TouchEntity* lastClicked) const {
  _touchEntityInterface->setLastClicked(lastClicked);
}
