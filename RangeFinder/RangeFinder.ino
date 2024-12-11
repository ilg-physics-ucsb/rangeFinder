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

#define PIN_START 18      // Green Button
#define PIN_STOP 9        // Red Button
#define PIN_UP 19         // Blue Button
#define PIN_DOWN 7        // Yellow Button
#define PIN_INTERRUPT 10  // Interrupt for the Range Finder

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
VL53L4CX rangeFinder(&Wire, 11);

volatile bool interruptFlag = 0;
VL53L4CX_MultiRangingData_t rangeFinderData;
VL53L4CX_MultiRangingData_t *pRangeFinderData = &rangeFinderData;
uint8_t dataFlag = 0 ;
int dataStatus = 0 ;
int objectNum = 0 ;
float data = 0 ;

bool displayFlag = 0 ;
char buff[7] ;

void interruptRangeFinder() {
  interruptFlag = 1;
}


// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200) ;

  // initialize GPIO pins
  pinMode(PIN_START, INPUT_PULLUP) ;
  pinMode(PIN_STOP,  INPUT_PULLUP) ;
  pinMode(PIN_UP,    INPUT_PULLUP) ;
  pinMode(PIN_DOWN,  INPUT_PULLUP) ;
  pinMode(PIN_LED,   OUTPUT) ;
  attachInterrupt(PIN_INTERRUPT, interruptRangeFinder, FALLING) ;

  // Start up the rangefinder in interrupt mode
  Wire.begin() ;
  rangeFinder.begin() ;
  rangeFinder.VL53L4CX_Off() ;
  rangeFinder.InitSensor(0x12) ;
  rangeFinder.VL53L4CX_SetDistanceMode(VL53L4CX_DISTANCEMODE_LONG) ;
  rangeFinder.VL53L4CX_SetMeasurementTimingBudgetMicroSeconds(50000) ;
  rangeFinder.VL53L4CX_StartMeasurement() ;


  // Start up display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay() ;
  display.setTextSize(1) ;                             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK) ; // Draw white text
  display.setCursor(0, 0) ;                            // Start at top-left corner
  display.display() ;
}

// the loop function runs over and over again forever
void loop() {
  //Read data from rangeFinder if interrupt triggered
  if (interruptFlag){
    interruptFlag = 0 ;
    digitalWrite(PIN_LED, HIGH) ;

    dataStatus = rangeFinder.VL53L4CX_GetMeasurementDataReady(&dataFlag) ;
    // Now actually read the data 
    //Checks to assure the status is 0 and flag is 1
    if (!dataStatus && dataFlag){
      // Retreive data from the device
      dataStatus = rangeFinder.VL53L4CX_GetMultiRangingData(pRangeFinderData) ;
      objectNum = pRangeFinderData->NumberOfObjectsFound ;

      // Iterate over every object found to get the 1 data point wanted 
      for (int j=0; j<objectNum; j++){
        if (pRangeFinderData->RangeData[j].RangeStatus != 0){
          data = pRangeFinderData->RangeData[j].RangeMilliMeter ; 
          break;
        }
      }

      // Start a new sensor measurement now
      if (dataStatus == 0) {
        dataStatus = rangeFinder.VL53L4CX_ClearInterruptAndStartMeasurement();
      }

      displayFlag = 1 ;
      digitalWrite(PIN_LED, LOW) ;
    }
  }

  if (displayFlag){
    displayFlag = 0 ;

    // Format the number to show
    sprintf(buff, "%06f", data) ;
    Serial.println(data) ;

    display.setCursor(0, 0) ;                            // Start at top-left corner
    display.print(buff) ;
    display.display() ;
  }
}