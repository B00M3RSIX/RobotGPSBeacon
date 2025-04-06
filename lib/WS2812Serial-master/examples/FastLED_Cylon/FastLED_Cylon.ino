// FastLED Cylon Example, using Non-Blocking WS2812Serial

#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 64
#define DATA_PIN 1

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() {
	Serial.begin(57600);
	Serial.println("resetting");
	LEDS.addLeds<WS2812SERIAL,DATA_PIN,RGB>(leds,NUM_LEDS);
	LEDS.setBrightness(84);
}

void loop() {
//Some code
	
}
