#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <time.h>

#include "configuration.h"

const int EPOCH_1_1_2019 = 1546300800;

Adafruit_NeoMatrix matrix =
  Adafruit_NeoMatrix(
    TILE_WIDTH, 8, TILE_HORIZ_COUNT, 1, PIN,
    NEO_TILE_TOP + NEO_TILE_LEFT +
    NEO_TILE_ROWS + NEO_TILE_PROGRESSIVE +
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
    NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
    NEO_GRB + NEO_KHZ800);

uint8_t brightness;
void set_brightness(uint8_t br) {
  brightness = br;
  matrix.setBrightness(brightness);
}

time_t get_time_from_internet() {
  set_status("time...");

  configTime(LOCAL_TIMEZONE_OFFSET_SECONDS,
            LOCAL_DST_OFFSET_SECONDS,
            "pool.ntp.org");

  time_t now = time(nullptr);
  while (now < EPOCH_1_1_2019) {
    delay(500);
    now = time(nullptr);
  };
  return now;
}

// We've acquired the current time from an external source.
// Recalculate how much time is left in the countdown.
bool clock_stopped = false;
int days = 0, hours = 0, minutes = 1, seconds = 0;
void update_countdown(time_t now) {
  time_t remaining = COUNTDOWN_TIMESTAMP - now;
  days = remaining / (24 * 60 * 60);
  remaining -= days * 24 * 60 * 60;
  hours = remaining / (60 * 60);
  remaining -= hours * 60 * 60;
  minutes = remaining / 60;
  remaining -= minutes * 60;
  seconds = remaining;
}

// Utility, spit out string to display and serial.
void set_status(const char *s) {
  matrix.setCursor(center_offset(s), 0);
  matrix.fillScreen(0);
  matrix.setTextColor(matrix.Color(0, 255, 0));
  matrix.print(s);
  matrix.show();
  Serial.printf("Status: %s\n", s);
}

unsigned long next_millis_update = 0;
void update_current_time_and_countdown() {
  unsigned long millis_now = millis();
  if (millis_now  > next_millis_update) {
    next_millis_update = millis_now + 1000 * 60 * 60 * 17 + 12345;
    update_countdown(get_time_from_internet());
  }
}

// Has one wall-clock second elapsed?
unsigned long time_now = 0;
unsigned long time_last = 0;
int last_second;
bool has_one_second_passed() {
  time_now = millis() / 1000;
  if (time_now - time_last != last_second) {
    last_second = time_now - time_last;
    return true;
  }
  return false;
}

// A wall-clock second has passed. Advance the countdown by
// one second.
//
// Returns true when the minute changes.
bool advance_clock() {
  if (--seconds < 0) {
    seconds = 59;
    if (--minutes < 0) {
      minutes = 59;
      if (--hours < 0) {
        hours = 23;
        if (--days < 0) {
          clock_stopped = true;
        }
      }
    }
    return true;
  }
  return false;
}

char time_display[32];
int draw_time_component(int x, int val) {
  matrix.setCursor(x, 0);
  sprintf(time_display, "%02d", val);
  matrix.print(time_display);
  return 6 * 2;
}

char divider_display[8] = ":";
int draw_divider(int x, bool should_draw) {
  if (should_draw) {
    matrix.setCursor(x - 2, 0);
    matrix.print(divider_display);
  }
  return 2;
}

// Set display color according to urgency of countdown.
void update_colors() {
  if (days < 7) {
    matrix.setTextColor(matrix.Color(255, 0, 0));
    return;
  }
  if (days < 14) {
    matrix.setTextColor(matrix.Color(255, 255, 0));
    return;
  }
  if (days < 21) {
    matrix.setTextColor(matrix.Color(0, 255, 0));
    return;
  }
  matrix.setTextColor(matrix.Color(0, 0, 255));
}

#define SSID_COUNT (sizeof(SSIDS) / sizeof(SSIDS[0]))

bool scan_wifi(const char **ssid, const char **passphrase) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  int n = WiFi.scanNetworks();
  Serial.printf("Network scan found %d\n", n);
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < SSID_COUNT; ++j) {
      if (strcmp(SSIDS[j], WiFi.SSID(i).c_str()) == 0) {
        *ssid = SSIDS[j];
        *passphrase = PASSPHRASES[j];
        return true;
      }
    }
  }
  return false;
}

void set_up_wifi() {
  const char *ssid, *passphrase;

// Don't bother scanning if we have only one
// network we want to connect to
  if (SSID_COUNT > 1) {
    set_status("scan...");
    do {
      if (scan_wifi(&ssid, &passphrase)) {
        set_status(ssid);
        break;
      } else {
        delay(5000);
      }
    } while (true);
  } else {
    ssid = SSIDS[0];
    passphrase = PASSPHRASES[0];
  }

  set_status("wifi...");
  WiFi.begin(ssid, passphrase);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  set_status(WiFi.localIP().toString().c_str());
}

String ota_url;
void setup() {
  ota_url = String(OTA_URL_PREFIX) + ESP.getSketchMD5();
  matrix.begin();
  matrix.setTextWrap(false);
  set_brightness(128);

  Serial.begin(115200);
  Serial.printf("\nBuild date %s time %s\n", __DATE__, __TIME__);
  Serial.printf("SSIDs configured: %d\n", SSID_COUNT);
  Serial.printf("OTA %s\n", ota_url.c_str());

  set_up_wifi();
}

void update_brightness() {
  time_t now = time(nullptr);
  struct tm ts = *localtime(&now);

  if (ts.tm_hour > DAY_HOUR_START && ts.tm_hour < DAY_HOUR_END) {
    set_brightness(DAY_BRIGHTNESS);
  } else {
    set_brightness(NIGHT_BRIGHTNESS);
  }
}

void draw_clock(bool should_draw_colons) {
  int x_cursor = 5;

  if (!clock_stopped) {
    x_cursor += draw_time_component(x_cursor, days);
    x_cursor += draw_divider(x_cursor, should_draw_colons);
    x_cursor += draw_time_component(x_cursor, hours);
    x_cursor += draw_divider(x_cursor, should_draw_colons);
    x_cursor += draw_time_component(x_cursor, minutes);
    x_cursor += draw_divider(x_cursor, should_draw_colons);
    x_cursor += draw_time_component(x_cursor, seconds);
  } else {
    set_status("Done.");
  }
}

int center_offset(String s) {
  return (TILE_HORIZ_COUNT * TILE_WIDTH - strlen(s.c_str()) * 6) / 2;
}

// Thanks https://gist.github.com/postspectacular/2a4a8db092011c6743a7
float fract(float x) {
  return x - int(x);
}

float mix(float a, float b, float t) {
  return a + (b - a) * t;
}

float step(float e, float x) {
  return x < e ? 0.0f : 1.0f;
}

float float_abs(float x) {
  if (x >= 0.0f) {
    return x;
  }
  return -x;
}

float* hsv_to_rgb(float h, float s, float b, float * rgb) {
  rgb[0] =
    b * mix(1.0f,
            constrain(float_abs(fract(h + 1.0f) * 6.0f - 3.0f) - 1.0f,
                      0.0f, 1.0f),
            s);
  rgb[1] =
    b * mix(1.0f,
            constrain(float_abs(fract(h + 0.6666666f) * 6.0f - 3.0f) - 1.0f,
                      0.0f, 1.0f),
            s);
  rgb[2] =
    b * mix(1.0f,
            constrain(float_abs(fract(h + 0.3333333f) * 6.0f - 3.0f) - 1.0f,
                      0.0f, 1.0f),
            s);
  return rgb;
}

float hue = 0.0f;
void draw_about() {
  String s = MESSAGE;
  float rgb[3];
  hsv_to_rgb(hue, 1.0f, 1.0f, rgb);
  hue += 0.01f;
  if (hue > 1.0f) {
    hue = 0.0f;
  }

  matrix.setTextColor(matrix.Color(
                        (int)((1.0 - rgb[0]) * 255),
                        (int)((1.0 - rgb[1]) * 255),
                        (int)((1.0 - rgb[2]) * 255)));
  matrix.setCursor(center_offset(s), 0);
  matrix.print(s);
}

// Call this once a second.
//
// TODO: this assumes the server is dumb and
// doesn't have the ability to return different HTTP
// result codes based on the version of the requesting
// client. So for now we hardcode the version into the
// OTA_URL path, relying on the server to return 404
// if there isn't anything yet to OTA to. Otherwise the
// client will OTA over and over to the same build.
int seconds_until_update_check = 15;
void check_for_update() {
  if (seconds_until_update_check-- == 0) {
    seconds_until_update_check = 15 * 60 + 7;
    t_httpUpdate_return r = ESPhttpUpdate.update(ota_url);
  }
}

// Call this once a second.
int seconds_until_program_change = 0;
bool show_about = false;
void check_for_program_change() {
  if (seconds_until_program_change-- == 0) {
    show_about = !show_about;
    if (show_about) {
      seconds_until_program_change = 13;  // prime number
    } else {
      seconds_until_program_change = 30;
    }
  }
}

unsigned long turn_colons_off_millis = 0;
void loop() {
  unsigned long now = millis();

  update_current_time_and_countdown();

  if (has_one_second_passed()) {
    if (!clock_stopped && advance_clock()) {
      // TODO
    }

    // Turn the colons on for some amount of time
    turn_colons_off_millis = now + 500;

    check_for_update();
    check_for_program_change();

    update_colors();
    update_brightness();
  }

  matrix.fillScreen(0);
  if (show_about) {
    draw_about();
  } else {
    draw_clock(turn_colons_off_millis != 0);
  }
  if (turn_colons_off_millis < now) {
    turn_colons_off_millis = 0;
  }

  matrix.show();

  delay(50);
}
