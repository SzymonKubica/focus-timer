#include "action_button.hpp"
#include "seven_segment_display/seven_segment_display.hpp"

#include <Arduino.h>
#include <Wire.h>

/*
 * We want to update the display every second, while polling for the user
 * input every 10ms. The ITERATIONS_PERIOD is needed to determine when we are
 * supposed to update the digit shown on the display.
 */
const int DISPLAY_UPDATE_INTERVAL = 1000;
const int POLLING_INTERVAL = 10;
const int ITERATIONS_PERIOD = DISPLAY_UPDATE_INTERVAL / POLLING_INTERVAL;

/*
 * Statically initialize all 'peripherals' of the focus timer.
 */
ActionButton button;
SevenSegmentDisplay display;

class FocusTimerState {
  /**
   * State variable incremented on each loop iteration. This is required to
   * determine how many iterations have passed since the last time we updated
   * the display.
   */
  int iterations_since_last_display_update = 0;

public:
  /**
   * Timestamp measured in terms of millis() when we started counting.
   */
  int counting_start_timestamp;
  /**
   * Tracks whether we are currently counting or if the focus timer is paused.
   */
  bool is_counting = false;

  bool iterations_period_reached() {
    return iterations_since_last_display_update % ITERATIONS_PERIOD == 0;
  }
  void increment_iteration() {
    iterations_since_last_display_update =
        (iterations_since_last_display_update + 1) % 1000;
  }
};

/*
 * Stores the logical state of the focus timer.
 */
FocusTimerState state;

void setup() {
  Serial.begin(115200);
  delay(500);

  // Initialize all 'peripherals'
  display.setup();
  button.setup();

  Serial.println("Focus Timer is ready!");
  state.counting_start_timestamp = millis();
}

/**
 * Helper function to convert elapsed seconds into a format that can be
 * displayed on the seven segement display as the number of minutes and seconds.
 */
int get_minutes_and_seconds(int elapsed_seconds) {
  int minutes = elapsed_seconds / 60;
  int seconds = elapsed_seconds % 60;
  return minutes * 100 + seconds;
}

void loop() {
  if (button.poll_for_input()) {
    Serial.println("Button press registered.");
    if (!state.is_counting) {
      Serial.println("Starting the counter.");
      state.counting_start_timestamp = millis();
    }
    state.is_counting = !state.is_counting;
  }

  if (state.is_counting && state.iterations_period_reached()) {
    int elapsed = (millis() - state.counting_start_timestamp) / 1000;
    display.set_number(get_minutes_and_seconds(elapsed));
  }
  state.increment_iteration();

  // We need to refresh the display even if the number wasn't updated. This is
  // required because of how the seven segement display works, it needs to
  // constantly change the segments that light up to allow for lighting up
  // each of the segments individually.
  display.refresh();
  // We wait between iterations to ensure that we aren't polling constantly
  // as that would be wasteful.
  delay(POLLING_INTERVAL);
}
