#pragma once

#define ACTION_BUTTON 15
#define PRESSED 1
#define NOT_PRESSED 0

/**
 * Action button is responsible for registering user input and starting/stopping
 * the focus timer sessions.
 */
class ActionButton {
private:
  int button_state = NOT_PRESSED;

public:
  void setup();
  bool poll_for_input();
};
