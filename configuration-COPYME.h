// To do OTAs, first run the following in the sketch directory:
//
// openssl genrsa -out private.key 2048
// openssl rsa -in private.key -outform PEM -pubout -out public.key
//
// Save a copy of private.key somewhere safe.

// The pin on the ESP8266 that's connected to the data-in (DIN)
// input on the WS2812 (a.k.a. Adafruit NeoPixel) lights
#define PIN D4

// Get this value from https://www.unixtimestamp.com/
// The timezone is UTC, but it'll be interpreted correctly.
//
// July 4, 2032, 12:00:00 (UTC): John Connor killed by T-850
// 1972555200

// Compare with a site like
// https://www.timeanddate.com/countdown/create
// to make sure it's counting down to the same timestamp.
#define COUNTDOWN_TIMESTAMP (1972555200)

// Occasionally displayed instead of countdown
#define MESSAGE "Terminated"

// If you put a signed Arduino bin at this URL, with the
// current bin's MD5 sum appended, then the firmware will
// OTA to that image.
//
#define OTA_URL_PREFIX "http://example.com/esp8266-ota/rgb-dash-"

// 255 = max brightness, 0 = turned off
#define DAY_BRIGHTNESS (128)
#define NIGHT_BRIGHTNESS (32)

// Which hour of the day to start/stop the DAY_BRIGHTNESS
// value
#define DAY_HOUR_START (6)
#define DAY_HOUR_END (18)

// Summer 2019 in California is UTC-7
#define LOCAL_TIMEZONE_OFFSET_SECONDS (-8 * 60 * 60)

// Right now it's Daylight Saving Time in California
#define LOCAL_DST_OFFSET_SECONDS (1 * 60 * 60)

// Read the Adafruit NeoMatrix docs for explanation
#define TILE_WIDTH (32)
#define TILE_HEIGHT (8)
#define TILE_HORIZ_COUNT (2)
#define TILE_VERT_COUNT (1)

// Add as many APs here as you want. You might want to do this
// so that you can move your sign around, maybe between work
// and home, without needing to reflash each time.
//
const char *SSIDS[] = {
  "my-home-ssid",
  "my-work-ssid",
};

const char *PASSPHRASES[] = {
  NULL,  // This means no password for my-home-ssid
  "my-work-ap-pw"
};
