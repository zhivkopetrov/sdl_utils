#ifndef SDL_UTILS_TOUCHENTITY_H_
#define SDL_UTILS_TOUCHENTITY_H_

/*
 * TouchEntity.h
 *
 *  Brief: used to mark a class than inherits this class as a TouchEntity.
 *
 *         Motivation: Engine needs to remember, which was the last pressed
 *         TouchEntity object in order to perform correct event actions.
 *
 *         Event newly selected TouchEntity will have to operate with one of
 *         the following actions:
 *              > PRESS;
 *              > DRAG;
 *              > RELEASE;
 *
 *         On selecting a new TouchEntity that is different than the
 *         previously selected one an action onLeave() will be invoked for the
 *         old TouchEntity.
 */

// System headers

// Other libraries headers

// Own components headers

// Forward Declarations
class InputEvent;
struct Rectangle;

class TouchEntity {
 public:
  virtual ~TouchEntity() = default;

  /** @brief used to check whether event is inside or not
   *                                       current frameRectangle or not
   *
   *  @param const InputEvent & - engine's InputEvent
   *
   *  @returns bool             - isInside or not
   * */
  virtual bool containsEvent(const InputEvent& e) = 0;

  /** @brief invoked when event was PRESSED inside this TouchEntity
   *         and event leaves the borders of the TouchEntity when no
   *         RELEASE action was performed inside of the same TouchEntity
   *
   *  @param const InputEvent & - engine's InputEvent
   * */
  virtual void onLeave(const InputEvent& e) = 0;

  /** @brief invoked when event was PRESSED inside this TouchEntity
   *         and event leaves the borders of the TouchEntity when no
   *         RELEASE action was performed inside of the same TouchEntity.
   *         While still action is PRESSED we then again return in the
   *         borders of this TouchEntity.
   *
   *  @param const InputEvent & - engine's InputEvent
   * */
  virtual void onReturn(const InputEvent& e) = 0;

  /** @brief used to lock this TouchEntity for incoming InputEvent
   * */
  virtual void lockInput() = 0;

  /** @brief used to unlock this TouchEntity for incoming InputEvent
   * */
  virtual void unlockInput() = 0;

  /** @brief used to lock whether this TouchEntity for locked for
   *                                                          InputEvents
   *
   *  @return bool - is TouchEntity locked or not
   * */
   bool isInputUnlocked() const { return _isInputUnlocked; }

  /** Points to button's event rectangle in order to check whether
   *  InputEvent is inside or not.
   * */
  const Rectangle* touchEntityEventRect = nullptr;

 protected:
  bool _isInputUnlocked = true;
};

#endif /* SDL_UTILS_TOUCHENTITY_H_ */
