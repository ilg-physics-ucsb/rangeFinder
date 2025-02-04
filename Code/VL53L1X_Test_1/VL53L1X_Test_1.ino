/**
 ******************************************************************************
 * @file    VL53L1_Sat_HelloWorld_Interrupt.ino
 * @author  SRA
 * @version V1.0.0
 * @date    30 July 2020
 * @brief   Arduino test application for the STMicrolectronics VL53L1
 *          proximity sensor satellite based on FlightSense.
 *          This application makes use of C++ classes obtained from the C
 *          components' drivers.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2020 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */


//On some boards like the Arduino Uno the pin used by the sensor to raise interrupts (A2)
//can't be mapped as an interrupt pin. For this this reason this sketch will not work
//unless some additional cabling is done and the interrupt pin is changed.
/*
 * To use this sketch you need to connect the VL53L1 satellite sensor directly to the Nucleo board with wires in this way:
 * pin 1 (Interrupt) of the VL53L1 satellite connected to pin A2 of the Nucleo board 
 * pin 2 (SCL_I) of the VL53L1 satellite connected to pin D15 (SCL) of the Nucleo board with a Pull-Up resistor of 4.7 KOhm
 * pin 3 (XSDN_I) of the VL53L1 satellite connected to pin A1 of the Nucleo board
 * pin 4 (SDA_I) of the VL53L1 satellite connected to pin D14 (SDA) of the Nucleo board with a Pull-Up resistor of 4.7 KOhm
 * pin 5 (VDD) of the VL53L1 satellite connected to 3V3 pin of the Nucleo board
 * pin 6 (GND) of the VL53L1 satellite connected to GND of the Nucleo board
 * pins 7, 8, 9 and 10 are not connected.
 */
/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>
#include <Wire.h>
#include <vl53l1x_class.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>


#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif
#define LedPin LED_BUILTIN

#define PIN_INTERRUPT 7   // Interrupt for the Range Finder

VL53L1X rangeFinder(&Wire, 9);

volatile int interruptFlag=0;
uint16_t dataRaw = 0 ;


void interruptRangeFinder(){
    interruptFlag=1;
}

void setup(){
    pinMode(LedPin, OUTPUT);
    pinMode(PIN_INTERRUPT, INPUT_PULLUP);
    attachInterrupt(PIN_INTERRUPT, interruptRangeFinder, RISING);

    // Initialize serial for output.
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("Starting...");

    // Initialize I2C bus.
    Wire.begin();
    rangeFinder.begin();
    rangeFinder.VL53L1X_Off();
    rangeFinder.InitSensor(0x29) ;
    rangeFinder.VL53L1X_SetROI(4, 4) ;
    rangeFinder.VL53L1X_SetTimingBudgetInMs(50) ;
    rangeFinder.VL53L1X_StartRanging() ;
 
}

void loop(){
  if (interruptFlag){
    interruptFlag=0;
    digitalWrite(LedPin, HIGH);

    rangeFinder.VL53L1X_GetDistance(&dataRaw) ;
    Serial.println(dataRaw) ;

    rangeFinder.VL53L1X_ClearInterrupt() ;
    digitalWrite(LedPin, LOW);
  }
}
