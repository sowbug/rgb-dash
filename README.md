RGB Dashboard
=

An LED countdown sign based on NeoMatrix-compatible LED tiles and
ESP8266.

You supply a timestamp in the future, and a short text description of
the meaning of the milestone, and this sketch will display a
01:23:45:59 countdown (1 day, 23 hours, 45 minutes, 59 seconds)
alternating with a rainbow-pulsing display of the text so that your
audience knows what you're counting down to. When it reaches the
milestone, it displays "Done."

The sign connects to time.nist.gov to get the current time. It
reconnects about once a day so the countdown remains fairly accurate.

The sketch can take multiple SSID/passphrase sets so that you can
develop your sign at home and then move it to its permanent location
without having to flash it again with the second network
configuration.

Howto
==

1. Get an Arduino IDE setup and successfully run the Blink sketch on
   your ESP8266. I used a [WEMOS D1
   Mini](https://wiki.wemos.cc/products:d1:d1_mini).

2. Connect your LED tile(s) to the ESP8266. V+ and GND go to power,
   and DIN (a.k.a. Data In) to one of the GPIO pins on the ESP8266. I
   connected DIN to D4 on my D1 Mini because D4 is right next to 5V
   and GND.

3. If you're using more than a very small matrix of LEDs, then you
   must also provide ample 5V power to them, as your PC's USB port
   probably provides only about 500mA. Estimate 50mA per pixel for the
   whole matrix at full-brightness white.

4. Check out this sketch and load it into the IDE. All the libraries
   you'll need are in the Arduino IDE's library manager. You'll need
   Adafruit GFX Library, Adafruit NeoMatrix, and Adafruit NeoPixel.

5. Make a copy of configuration-COPYME.h named configuration.h.

6. Change whatever you need to in configuration.h. You'll definitely
   need to change SSIDS and PASSPHRASES to your local wifi network,
   and the TILE_ values should match your NeoMatrix hardware
   configuration.

7. Build and upload the sketch.

8. If all's well, you can now disconnect the sign from your PC and
   power it externally.
