#include "action_button.hpp"
#include <Arduino.h>

void ActionButton::setup() { pinMode(ACTION_BUTTON, INPUT_PULLUP); }

/**
 * Polls the button for input. Returns true if the button was just pressed,
 * false otherwhise. Note that we need to do this elaborate debounce handling to
 * ensure that we don't accidentally register multiple button presses when the
 * user only intended to press once. We do this by 'caching' the button state in
 * a field of the ActionButton class and checking if the read value is actually
 * different from that cached field.
 */
bool ActionButton::poll_for_input() {
  int pin_state = digitalRead(ACTION_BUTTON);
  delay(10); // debounce filter
  if (button_state != pin_state) {
    bool test = pin_state == PRESSED && button_state == NOT_PRESSED;
    button_state = pin_state;
    return test;
  }
  return false;
}
