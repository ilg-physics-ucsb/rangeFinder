#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <vl53l1x_class.h>
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

#define PIN_START 10      // Green Button
#define PIN_STOP 13       // Red Button
#define PIN_TIME 11       // Blue Button
#define PIN_RATE 12       // Yellow Button
#define PIN_INTERRUPT 7   // Interrupt for the Range Finder
#define PIN_SHUTDOWN 9    // Shutdown pin for the Range Finder

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
VL53L1X rangeFinder(&Wire, PIN_SHUTDOWN);

uint8_t dataFlag = 0 ;
int dataStatus = 0 ;
int objectNum = 0 ;
unsigned long dataRawTime = 50 ; // Time for the sensor to average over 
uint16_t dataRaw = 0 ;
float data = 0 ;
float dataLast = 0 ;
float dataLastLast = 0 ;

int dataRecordingIntervalIndex = 0 ;
unsigned long dataRecordingInterval[] = {10000, 15000, 20000, 30000, 5000} ; //How long to record data in millis
unsigned long dataRecordingTime = 0 ;
unsigned long dataRecordingTimeStart = 0 ;
char dataRecordingIntervalBuff[5] ;


int dataFilterIntervalIndex = 0 ;
unsigned long dataFilterInterval[] = {200, 500, 1000, 50, 100};              // Time for the us to filter the sensors data over
unsigned long dataFilterTime = 0 ;    
int dataFilterSamples = dataFilterInterval[dataFilterIntervalIndex]/dataRawTime ;
char dataFilterIntervalBuff[5] ;                          

float time = 0 ;         // Time of recorded data
float timeLast = 0 ;     // Last time for velocity
float timeLastLast = 0 ; // Last Last time for velocity

float pos = 0 ;             // Smoothed data in m
float posLast = 0 ;         // Last position for filtering
float vel = 0 ;             // Smoothed data of velocity in m/s
float velLast = 0 ;         // Last velocity for filtering
float alpha = 0.5 ;        // Factor for data smoothing 
float alphaInv = 1-alpha ;  // Precalculating 
char posBuff[5] ;
char velBuff[8] ;
char timeBuff[6] ;

bool dataNewFlag = 0 ;
int dataArrayIndex = 0 ;
float timeArray[1000] ;
float posArray[1000] ;
float velArray[1000] ;
int keyboardDelay = 40 ;

int posPixelX = 0 ;
int posPixelY = 0 ;

unsigned long displayInterval = 200 ;
unsigned long displayTime = 0 ;

unsigned long buttonDebounceWait   = 20 ;  // In millis
unsigned long buttonStartDebounce  = 0 ;
unsigned long buttonStopDebounce   = 0 ;
unsigned long buttonRateDebounce   = 0 ;
unsigned long buttonTimeDebounce   = 0 ;
bool buttonStartToggle    = 0 ;
bool buttonStopToggle     = 0 ;
bool buttonRateToggle     = 0 ;
bool buttonTimeToggle     = 0 ;
bool buttonStartFlag      = 0 ;
bool buttonStopFlag       = 0 ;
bool buttonRateFlag       = 0 ;
bool buttonTimeFlag       = 0 ;

int state = 0 ;
int stateLast = 0 ;
bool displayFlag = 0 ;
volatile bool interruptFlag = 0;
unsigned long t0 = 0 ;


void interruptRangeFinder() {
  interruptFlag = 1;
}


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize GPIO pins
  pinMode(PIN_START, INPUT_PULLUP) ;
  pinMode(PIN_STOP,  INPUT_PULLUP) ;
  pinMode(PIN_TIME,    INPUT_PULLUP) ;
  pinMode(PIN_RATE,  INPUT_PULLUP) ;
  pinMode(PIN_LED,   OUTPUT) ;
  attachInterrupt(PIN_INTERRUPT, interruptRangeFinder, RISING) ;

  // Start up the rangefinder in interrupt mode
  Wire.begin();
  rangeFinder.begin();
  rangeFinder.VL53L1X_Off();
  rangeFinder.InitSensor(0x29) ;
  rangeFinder.VL53L1X_SetROI(4, 4) ;
  rangeFinder.VL53L1X_SetTimingBudgetInMs(dataRawTime) ;
  rangeFinder.VL53L1X_StartRanging() ;

  // Start up display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS) ;
  display.setRotation(2) ;
  display.setTextSize(1) ;                             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK) ; // Draw white text
  initDisplay(state) ;

  //Initialize Keyboard
  Keyboard.begin() ;
}

void loop() {
  // Update interactives
  updateButtons() ;
  updateRangeFinder() ;

  // Main state machine for handling data
  stateLast = state ;
  switch(state) {
    case 0:
      // Idle mode: Display distance and react to buttons

      // Update position so you know the sensor is working
      if (millis() - displayTime >= displayInterval){
        displayTime = millis() ;
        display.setTextSize(2) ;
        display.setCursor(22, 0) ;
        formatPos(pos) ;
        display.print(posBuff) ;
        display.print("m") ;
        display.setTextSize(1) ;
        displayFlag = 1 ;

      } 

      // Start recording data is green button pressed
      if (buttonStartFlag){
        state = 1 ;
        initDisplay(state) ;
        dataRecordingTimeStart = millis() ;
      }

      // Adjust the length of data collection
      if (buttonTimeFlag){
        buttonTimeFlag = 0 ;
        dataRecordingIntervalIndex = (dataRecordingIntervalIndex+1)%5 ;
        formatRecordingInterval() ;
        display.setCursor(94, 24) ;
        display.print(dataRecordingIntervalBuff) ;
        displayFlag = 1 ;
      }

      // Adjusts the rate at which data is taken
      if (buttonRateFlag){
        buttonRateFlag = 0 ;
        dataFilterIntervalIndex = (dataFilterIntervalIndex+1)%5 ;
        formatFilterInterval() ;
        display.setCursor(30, 24) ;
        display.print(dataFilterIntervalBuff) ;
        dataFilterSamples = dataFilterInterval[dataFilterIntervalIndex]/dataRawTime ;
        displayFlag = 1 ;
      }

      // Need to handle buttons even if not used
      if (buttonStopFlag){
        buttonStopFlag = 0 ;
      }

      // Need to handle data New flag even if not used
      if (dataNewFlag){
        dataNewFlag = 0 ;
      }
    break;
    case 1:
      // Recording mode: Display graph of distance and only react to red button

      if (dataNewFlag){
        dataNewFlag = 0 ;
        // If new data available record it
        // Records all data, can be parsed for sparser data when recorded
        timeArray[dataArrayIndex] = time ;
        posArray[dataArrayIndex] = pos ;
        velArray[dataArrayIndex] = vel ;
        dataArrayIndex ++ ;

        posPixelX = 128*time*1000/dataRecordingInterval[dataRecordingIntervalIndex] ;  // Divided by max time on graph
        posPixelY = 31 - 32*pos/2 ;                                               // Divided by max position showable

        display.drawPixel(posPixelX, posPixelY, SSD1306_WHITE) ;
        displayFlag = 1 ;

      }


      // End data recording and write is all to the keyboard if its time
      if (millis() - dataRecordingTimeStart >= dataRecordingInterval[dataRecordingIntervalIndex]){
        state = 2 ;
        initDisplay(state) ;
      }

      // Also end recording if red button pressed
      if (buttonStopFlag){
        buttonStopFlag = 0 ;
        state = 2 ;
        initDisplay(state) ;
      }

      // Need to handle buttons even if not used
      if (buttonStartFlag){
        buttonStartFlag = 0 ;
      }
      if (buttonRateFlag){
        buttonRateFlag = 0 ;
      }
      if (buttonTimeFlag){
        buttonTimeFlag = 0 ;
      }
    break;
    case 2:
      // Write mode: Triggered on end of recording mode, blocking function to write data to keyboard
      // Blocking function so no need to handle flags 

      for (int i=0; i<dataArrayIndex; i+=dataFilterSamples){
        Keyboard.print(timeArray[i], 4) ;
        delay(keyboardDelay) ;
        Keyboard.press('\t') ;
        delay(keyboardDelay) ;
        Keyboard.release('\t') ;
        Keyboard.print(posArray[i], 4) ;
        delay(keyboardDelay) ;
        Keyboard.press('\t') ;
        delay(keyboardDelay) ;
        Keyboard.release('\t') ;
        Keyboard.print(velArray[i], 4) ;
        delay(keyboardDelay) ;
        Keyboard.press('\n') ;
        delay(keyboardDelay) ;
        Keyboard.release('\n') ;

      }


      state = 0 ;
      dataArrayIndex = 0 ;
      initDisplay(state) ;
    break;
  }

  // Write to the display when necessary 
  updateDisplay() ;
}


//---------------------------------------------------------------------------------------------------------------------
// Update Functions 
//---------------------------------------------------------------------------------------------------------------------

// Update the distanced detected 
void updateRangeFinder(){
  //Read data from rangeFinder if interrupt triggered
  if (interruptFlag){
    interruptFlag = 0 ;
    digitalWrite(PIN_LED, HIGH) ;

    //Read in the data
    rangeFinder.VL53L1X_GetDistance(&dataRaw) ;
    data = (float) dataRaw ;
    dataRecordingTime = millis() ;

    //Clear Interrupt for next data
    rangeFinder.VL53L1X_ClearInterrupt() ;
    digitalWrite(PIN_LED, LOW) ;

    // Calculate Filtered Position and velocity
    computeCoordinates(data) ;
    dataNewFlag = 1 ;
  }
}

// Update Buttons
void updateButtons(){
  //Check button states and set flags if they are pressed
  if (!digitalRead(PIN_START) && !buttonStartToggle){
    buttonStartToggle = 1 ; 
    buttonStartFlag = 1 ;
    buttonStartDebounce = millis() ;
  } else if (digitalRead(PIN_START) && buttonStartToggle && (millis() - buttonStartDebounce >= buttonDebounceWait)){
    buttonStartToggle = 0 ;
  }

  if (!digitalRead(PIN_STOP) && !buttonStopToggle){
    buttonStopToggle = 1 ; 
    buttonStopFlag = 1 ;
    buttonStopDebounce = millis() ;
  } else if (digitalRead(PIN_STOP) && buttonStopToggle && (millis() - buttonStopDebounce >= buttonDebounceWait)){
    buttonStopToggle = 0 ;
  }

  if (!digitalRead(PIN_RATE) && !buttonRateToggle){
    buttonRateToggle = 1 ; 
    buttonRateFlag = 1 ;
    buttonRateDebounce = millis() ;
  } else if (digitalRead(PIN_RATE) && buttonRateToggle && (millis() - buttonRateDebounce >= buttonDebounceWait)){
    buttonRateToggle = 0 ;
  }

  if (!digitalRead(PIN_TIME) && !buttonTimeToggle){
    buttonTimeToggle = 1 ; 
    buttonTimeFlag = 1 ;
    buttonTimeDebounce = millis() ;
  } else if (digitalRead(PIN_TIME) && buttonTimeToggle && (millis() - buttonTimeDebounce >= buttonDebounceWait)){
    buttonTimeToggle = 0 ;
  }
}

// Update the display
void updateDisplay(){
  // Only write to display when new text
  if (displayFlag){
    displayFlag = 0 ;
    display.display() ;
  }
}

//---------------------------------------------------------------------------------------------------------------------
// Utility Functions 
//---------------------------------------------------------------------------------------------------------------------

// Positive definite modulus
int mod(int x, int y){
  int val = x%y ;
  if (val < 0){
    val += y ;
  }
  return val ;
}

void initDisplay(int _state){
  // Clear the initialize a new screen
  // Used to switch cases or start up

  display.clearDisplay() ;
  switch(_state){
    case 0:
      // Display Distance
      display.setTextSize(2) ;
      display.setCursor(22, 0) ;
      formatPos(pos) ;
      display.print(posBuff) ;
      display.print("m") ;
      display.setTextSize(1) ;

      // Display options
      display.setCursor(0, 24) ;
      display.print("Rate:") ;
      formatFilterInterval() ;
      display.print(dataFilterIntervalBuff) ;

      display.setCursor(64, 24) ;
      display.print("Time:") ;
      formatRecordingInterval() ;
      display.print(dataRecordingIntervalBuff) ;
    break;
    case 1:
      // Setup the graph for displaying
      display.drawFastVLine(0, 0, 32, SSD1306_WHITE) ;
      display.drawFastHLine(0, 31, 128, SSD1306_WHITE) ;
    break;
    case 2:
      // Write Uploading for the blocking function 
      display.setTextSize(2) ;
      display.setCursor(10, 12) ;
      display.print("Uploading") ;
      display.setTextSize(1) ;
    break;
  }
  
  displayFlag = 1 ;
}

void computeCoordinates(int val){
  // Computes the position and velocity after every measurement
  // Eventually should use filtering (Kalman filter)
  time = (float) (dataRecordingTime - dataRecordingTimeStart)/1000 ;
  pos = (float) alpha*( (data + dataLast)/2000 ) + alphaInv*posLast ;
  vel = (float) alpha*( (data - dataLastLast)/(time - timeLastLast)/1000 ) + alphaInv*velLast ;

  // Set last values for comparing to next time
  dataLastLast = dataLast ;
  dataLast = data ; 
  timeLastLast = timeLast ;
  timeLast = time ;
  posLast = pos ;
  velLast = vel ;
  return ;
}

// Format the position as a string
void formatPos(float _position){
  posBuff[0] = String( (int) floor(abs(_position)) % 10 )[0] ;
  posBuff[1] = '.' ;
  posBuff[2] = String( (int) floor(abs(_position)*10) % 10 )[0] ;
  posBuff[3] = String( (int) floor(abs(_position)*100) % 10 )[0] ;
  posBuff[4] = String( (int) floor(abs(_position)*1000) % 10 )[0] ;
  return ;
}

// Format the velocity as a string
void formatVel(float _velocity){
  // Format the velocity number to show
  if (_velocity<0){
    velBuff[0] = '-' ;
  } else {
    velBuff[0] = '+' ;
  }
  velBuff[1] = String( (int) floor(abs(_velocity)) % 10 )[0] ;
  velBuff[2] = '.' ;
  velBuff[3] = String( (int) floor(abs(_velocity)*10) % 10 )[0] ;
  velBuff[4] = String( (int) floor(abs(_velocity)*100) % 10 )[0] ;
  velBuff[5] = String( (int) floor(abs(_velocity)*1000) % 10 )[0] ;
  velBuff[6] = String( (int) floor(abs(_velocity)*10000) % 10 )[0] ;
  velBuff[7] = String( (int) floor(abs(_velocity)*100000) % 10 )[0] ;
  return ;
}

// Format the time as a string
void formatTime(float _time){
  timeBuff[0] = String( (int) floor(_time/10) % 10 )[0] ;
  timeBuff[1] = String( (int) floor(_time) % 10 )[0] ;
  timeBuff[2] = '.' ;
  timeBuff[3] = String( (int) floor(_time*10) % 10 )[0] ;
  timeBuff[4] = String( (int) floor(_time*100) % 10 )[0] ;
  timeBuff[5] = String( (int) floor(_time*1000) % 10 )[0] ;
  return ;
}

// Format the options for printing
void formatFilterInterval(){
  float rate = (float) dataFilterInterval[dataFilterIntervalIndex] / 1000 ;
  dataFilterIntervalBuff[0] = String( (int) floor(rate) % 10 )[0] ;
  dataFilterIntervalBuff[1] = '.' ;
  dataFilterIntervalBuff[2] = String( (int) floor(rate*10) % 10 )[0] ;
  dataFilterIntervalBuff[3] = String( (int) floor(rate*100) % 10 )[0] ;
  dataFilterIntervalBuff[4] = 's' ;
  return ;
}

void formatRecordingInterval(){
  int time = dataRecordingInterval[dataRecordingIntervalIndex]/1000 ;
  dataRecordingIntervalBuff[0] = String(time / 10 )[0] ;
  dataRecordingIntervalBuff[1] = String(time % 10 )[0] ;
  dataRecordingIntervalBuff[2] = '.' ;
  dataRecordingIntervalBuff[3] = '0' ;
  dataRecordingIntervalBuff[4] = 's' ;
  return ;
}
