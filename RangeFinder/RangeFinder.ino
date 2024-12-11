#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <vl53l4cx_class.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define PIN_START A4      // Green Button
#define PIN_STOP 9        // Red Button
#define PIN_UP A5         // Blue Button
#define PIN_DOWN 7        // Yellow Button
#define PIN_INTERRUPT 10  // Interrupt for the Range Finder

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
VL53L4CX rangeFinder(&Wire, 11);

volatile bool interruptFlag = 0;

void interruptRangeFinder() {
  interruptCount = 1;
}


// the setup function runs once when you press reset or power the board
void setup() {
  // Start up display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay() ;
  display.setTextSize(1) ;                             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK) ; // Draw white text
  display.setCursor(0, 0) ;                            // Start at top-left corner
  display.print("Hello World!") ;
  display.display() ;

  // initialize buttons
  pinMode(PIN_START, INPUT_PULLUP) ;
  pinMode(PIN_STOP,  INPUT_PULLUP) ;
  pinMode(PIN_UP,    INPUT_PULLUP) ;
  pinMode(PIN_DOWN,  INPUT_PULLUP) ;
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second
}