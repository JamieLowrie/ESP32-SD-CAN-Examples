#include <FastLED.h>

#define NUM_LEDS 1
#define LED_PIN 8

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {
  leds[0] = CRGB::Green;
  FastLED.show();
  delay(500);

  leds[0] = CRGB::Black;  // off
  FastLED.show();
  delay(500);
}