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
#include <Keyboard.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define PIN_START 18      // Green Button
#define PIN_STOP 9        // Red Button
#define PIN_TIME 19       // Blue Button
#define PIN_SAMPLE 7      // Yellow Button
#define PIN_INTERRUPT 10  // Interrupt for the Range Finder

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
VL53L4CX rangeFinder(&Wire, 11);

volatile bool interruptFlag = 0;
VL53L4CX_MultiRangingData_t rangeFinderData;
VL53L4CX_MultiRangingData_t *pRangeFinderData = &rangeFinderData;
uint8_t dataFlag = 0 ;
int dataStatus = 0 ;
int objectNum = 0 ;
int dataRaw = 0 ;
int dataRawLast = 0 ;
int dataTime = 200 ; // Time for the sensor to average the distance over (in milliseconds)
float pos = 0 ;        // Smoothed data in m
float vel = 0 ;        // Smoother data of velocity in m/s

bool displayFlag = 0 ;
char posBuff[5] ;
char velBuff[6] ;

unsigned long buttonDebounceWait   = 20 ;  // In millis
unsigned long buttonStartDebounce  = 0 ;
unsigned long buttonStopDebounce   = 0 ;
unsigned long buttonSampleDebounce = 0 ;
unsigned long buttonTimeDebounce   = 0 ;

bool buttonStartToggle    = 0 ;
bool buttonStopToggle     = 0 ;
bool buttonSampleToggle   = 0 ;
bool buttonTimeToggle     = 0 ;

bool buttonStartFlag      = 0 ;
bool buttonStopFlag       = 0 ;
bool buttonSampleFlag     = 0 ;
bool buttonTimeFlag       = 0 ;



void interruptRangeFinder() {
  interruptFlag = 1;
}


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize GPIO pins
  pinMode(PIN_START, INPUT_PULLUP) ;
  pinMode(PIN_STOP,  INPUT_PULLUP) ;
  pinMode(PIN_TIME,    INPUT_PULLUP) ;
  pinMode(PIN_SAMPLE,  INPUT_PULLUP) ;
  pinMode(PIN_LED,   OUTPUT) ;
  attachInterrupt(PIN_INTERRUPT, interruptRangeFinder, FALLING) ;

  // Start up the rangefinder in interrupt mode
  Wire.begin() ;
  rangeFinder.begin() ;
  rangeFinder.VL53L4CX_Off() ;
  rangeFinder.InitSensor(0x12) ;
  rangeFinder.VL53L4CX_SetDistanceMode(VL53L4CX_DISTANCEMODE_LONG) ;
  rangeFinder.VL53L4CX_SetMeasurementTimingBudgetMicroSeconds(dataTime*1000) ;
  rangeFinder.VL53L4CX_StartMeasurement() ;


  // Start up display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS) ;

  display.clearDisplay() ;
  display.setTextSize(1) ;                             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK) ; // Draw white text
  display.setTextSize(2) ;
  display.setCursor(0, 0) ;                            // Start at top-left corner
  display.display() ;

  //Initialize Keyboard
  Keyboard.begin() ;
}

// the loop function runs over and over again forever
void loop() {
  updateRangeFinder() ;
  updateButtons() ;
  updateOptions() ;
  updateDisplay() ;
}
//---------------------------------------------------------------------------------------------------------------------
// Functions 
//---------------------------------------------------------------------------------------------------------------------

// Update the display
void updateDisplay(){
  // Only write to display when new text
  if (displayFlag){
    displayFlag = 0 ;

    // Format the number to show
    posBuff[0] = String( (int) floor(abs(pos)) % 10 )[0] ;
    posBuff[1] = '.' ;
    posBuff[2] = String( (int) floor(abs(pos)*10) % 10 )[0] ;
    posBuff[3] = String( (int) floor(abs(pos)*100) % 10 )[0] ;
    posBuff[4] = String( (int) floor(abs(pos)*1000) % 10 )[0] ;
    display.setCursor(0, 0) ;   
    display.print(posBuff) ;

    if (vel<0){
      velBuff[0] = '-' ;
    } else {
      velBuff[0] = '+' ;
    }
    velBuff[1] = String( (int) floor(abs(vel)) % 10 )[0] ;
    velBuff[2] = '.' ;
    velBuff[3] = String( (int) floor(abs(vel)*10) % 10 )[0] ;
    velBuff[4] = String( (int) floor(abs(vel)*100) % 10 )[0] ;
    velBuff[5] = String( (int) floor(abs(vel)*1000) % 10 )[0] ;
    display.setCursor(0, 16) ;   
    display.print(velBuff) ;
    display.display() ;
  }
}

// Update the distanced detected 
void updateRangeFinder(){
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
        if (pRangeFinderData->RangeData[j].RangeStatus == 0){
          dataRawLast = dataRaw ;
          dataRaw = pRangeFinderData->RangeData[j].RangeMilliMeter ; 

          pos = (float) dataRaw / 1000 ;
          vel = (float) (dataRaw - dataRawLast) / dataTime ;
          break;
        }
      }

      // Start a new sensor measurement now
      if (dataStatus == 0) {
        dataStatus = rangeFinder.VL53L4CX_ClearInterruptAndStartMeasurement();
      }
      digitalWrite(PIN_LED, LOW) ;
      displayFlag = 1 ;
    }
  }
}

// Update Buttons
void updateButtons(){
  //Check button states and set flags if they are pressed
  if (digitalRead(PIN_START) && !buttonStartToggle){
    buttonStartToggle = 1 ; 
    buttonStartFlag = 1 ;
    buttonStartDebounce = millis() ;
  } else if (!digitalRead(PIN_START) && buttonStartToggle && (millis() - buttonDebounceWait >= buttonStartDebounce)){
    buttonStartToggle = 0 ;
  }

  if (digitalRead(PIN_STOP) && !buttonStopToggle){
    buttonStopToggle = 1 ; 
    buttonStopFlag = 1 ;
    buttonStopDebounce = millis() ;
  } else if (!digitalRead(PIN_STOP) && buttonStopToggle && (millis() - buttonDebounceWait >= buttonStopDebounce)){
    buttonStopToggle = 0 ;
  }

  if (digitalRead({PIN_SAMPLE) && !buttonSampleToggle){
    buttonSampleToggle = 1 ; 
    buttonSampleFlag = 1 ;
    buttonSampleDebounce = millis() ;
  } else if (!digitalRead(PIN_SAMPLE) && buttonSampleToggle && (millis() - buttonDebounceWait >= buttonSampleDebounce)){
    buttonSampleToggle = 0 ;
  }

  if (digitalRead({PIN_TIME) && !buttonTimeToggle){
    buttonTimeToggle = 1 ; 
    buttonTimeFlag = 1 ;
    buttonTimeDebounce = millis() ;
  } else if (!digitalRead(PIN_SAMPLE) && buttonTimeToggle && (millis() - buttonDebounceWait >= buttonTimeDebounce)){
    buttonTimeToggle = 0 ;
  }
}

// Handle any functions of the buttons
void updateOptions(){
  // Do an action when any of the buttons are flagged
  if (buttonStartFlag){
    buttonStartFlag = 0 ;
    //Do Stuff
  } 

  if (buttonStopFlag){
    buttonStopFlag = 0 ;
    //Do Stuff
  } 

  if (buttonSampleFlag){
    buttonSampleFlag = 0 ;
    //Do Stuff
  }

  if (buttonTimeFlag){
    buttonTimeFlag = 0 ;
    //Do Stuff
  }
}