#include "action_button.hpp"
#include "ntp_time_provider/ntp_time_provider.hpp"
#include "seven_segment_display/seven_segment_display.hpp"
#include "wifi_provider/wifi_provider.hpp"

#include <Arduino.h>
#include <Wire.h>

/*
 * We want to update the display every second, while polling for the user
 * input every 1ms. The ITERATIONS_PERIOD is needed to determine when we are
 * supposed to update the digit shown on the display. We need to poll this often
 * because the seven segment display needs to be refreshed frequently to avoid
 * weird flickering.
 */
const int DISPLAY_UPDATE_INTERVAL = 1000;
const int POLLING_INTERVAL = 1;
const int ITERATIONS_PERIOD = DISPLAY_UPDATE_INTERVAL / POLLING_INTERVAL;

/*
 * Statically initialize all 'peripherals' of the focus timer.
 */
ActionButton button;
SevenSegmentDisplay display;

enum class TimerMode { COUNTING, IDLE, CLOCK };

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
  TimerMode mode = TimerMode::CLOCK;

  bool iterations_period_reached() {
    return iterations_since_last_display_update % ITERATIONS_PERIOD == 0;
  }
  void increment_iteration() {
    iterations_since_last_display_update =
        (iterations_since_last_display_update + 1) % ITERATIONS_PERIOD;
  }
};

/*
 * Stores the logical state of the focus timer.
 */
FocusTimerState state;

struct TimeHM {
  uint8_t hour;
  uint8_t minute;
};

TimeHM get_utc_hour_minute(uint32_t epoch) {
  TimeHM t;

  uint32_t second_modulo_day = epoch % 86400UL;

  t.hour = second_modulo_day / 3600;
  t.minute = (second_modulo_day % 3600) / 60;

  return t;
}

int get_hours_and_minutes(TimeHM time) { return time.hour * 100 + time.minute; }

void setup() {
  Serial.begin(115200);
  delay(500);

  // Initialize all 'peripherals'
  display.setup();
  button.setup();

  WifiProvider provider;
  provider.connect();

  TimeProvider time_provider;
  time_provider.setup();
  auto time = time_provider.get_current_time();

  Serial.print("Unix time: ");
  Serial.println(time);

  Serial.print("UTC time: ");
  Serial.println(String(get_utc_hour_minute(time).hour) + ":" +
                 String(get_utc_hour_minute(time).minute));

  display.set_number(get_hours_and_minutes(get_utc_hour_minute(time)));

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
    if (state.mode == TimerMode::IDLE || state.mode == TimerMode::CLOCK) {
      Serial.println("Starting the counter.");
      state.counting_start_timestamp = millis();
      state.mode = TimerMode::COUNTING;
    } else {
      state.mode = TimerMode::IDLE;
    }
  }

  if (state.mode == TimerMode::COUNTING && state.iterations_period_reached()) {
    int elapsed_millis = (millis() - state.counting_start_timestamp);
    int elapsed_seconds = elapsed_millis / 1000;
    display.set_number(get_minutes_and_seconds(elapsed_seconds));
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
