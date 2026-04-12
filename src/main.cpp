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

const int POLAND_TIMEZONE_OFFSET_HOURS = 2;

/*
 * Statically initialize all 'peripherals' of the focus timer.
 */
ActionButton button;
SevenSegmentDisplay display;
TimeProvider time_provider;

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
  long last_time_fetch_timestamp = 0;
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

  time_provider.setup();
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
    if (state.mode == TimerMode::CLOCK) {
      Serial.println("Starting the counter.");
      state.counting_start_timestamp = millis();
      state.mode = TimerMode::COUNTING;
    } else if (state.mode == TimerMode::COUNTING) {
      state.mode = TimerMode::IDLE;
    } else if (state.mode == TimerMode::IDLE) {
      // Force re-fetch of the time when toggled into the clock mode.
      state.last_time_fetch_timestamp = 0;
      state.mode = TimerMode::CLOCK;
    }
  }

  if (state.mode == TimerMode::COUNTING && state.iterations_period_reached()) {
    int elapsed_millis = (millis() - state.counting_start_timestamp);
    int elapsed_seconds = elapsed_millis / 1000;
    display.set_number(get_minutes_and_seconds(elapsed_seconds));
  }

  if (state.mode == TimerMode::CLOCK && state.iterations_period_reached()) {
    if (state.last_time_fetch_timestamp == 0 ||
        millis() - state.last_time_fetch_timestamp > 60000) {
      auto time = time_provider.get_current_time();
      state.last_time_fetch_timestamp = millis();

      Serial.print("Unix time: ");
      Serial.println(time);
      auto utc_time = get_utc_hour_minute(time);
      utc_time.hour += POLAND_TIMEZONE_OFFSET_HOURS;

      Serial.print("UTC time: ");
      Serial.println(String(utc_time.hour) + ":" + String(utc_time.minute));

      display.set_number(get_hours_and_minutes(utc_time));
    }
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
