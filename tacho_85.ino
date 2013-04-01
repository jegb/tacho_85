/**
 * My Tachometer based on Attiny85 and i2c LCD based on open source example
 *
 *  Copyright 2013 by Enrique Gallar  <no_spam@please.com>
 *
 * This file is part of some open source application.
 * 
 * Some open source application is free software: you can redistribute 
 * it and/or modify it under the terms of the GNU General Public 
 * License as published by the Free Software Foundation, either 
 * version 3 of the License, or (at your option) any later version.
 * 
 * Some open source application is distributed in the hope that it will 
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */
// from http://www.pyroelectro.com/tutorials/tachometer_rpm_arduino/software.html
// using this opto sensor http://www.fairchildsemi.com/ds/QR/QRD1113.pdf
//
// ATtiny pins : 
// SDA pin 5 (PB0) SCK pin 7(PB2), both with pull ups 4K7
// Sensor pin 6 (PB1) via internal pull-up for optocoupler
// Some edits done on the LCD library for ywrobot 1602 display to work with i2c well

#include <TinyWireM.h>                  // I2C Master lib for ATTinys which use USI
#include <LiquidCrystal_I2C.h>          // for LCD w/ GPIO MODIFIED for the ATtiny85
#include <PinChangeInterrupt.h>         // nice library for attiny handling interrupts

volatile unsigned long time = 0;
volatile unsigned long time_last = 0;
volatile unsigned int rpm_array[5] = {
  0,0,0,0,0}; // 5 samples averaged
#define sensor_pin PB1 // no need for external pullup, open collector from opto will do its job
unsigned long refreshInterval = 400; // in ms, for LCD refresh timing
unsigned long previousMillis = 0;
volatile unsigned int rpm = 0;
volatile unsigned int last_rpm = 0;

LiquidCrystal_I2C lcd(0x27,16,2);  // set address & 16 chars / 2 lines

void setup()
{
  //Digital Pin D1 Set As An Interrupt and pull-up internal set
  pinMode(sensor_pin, INPUT);
  digitalWrite(sensor_pin, HIGH);
  attachPcInterrupt(1,SENSOR_ISR,FALLING);  // Catch IR sensor on PB1 with pin change interrupt
  lcd.init();
  lcd.backlight();
  // we could handle with LCD 1x8 chars RPM00000
  lcd.print("Current RPM:"); // this remains static only refresh on second line
  delay(1000);
}

//Main Loop To Calculate RPM and Update LCD Display
void loop(){
  update_rpm(); // sensor ISR will look for time period, notice we also work at 1Mhz
  if (last_rpm - rpm !=  0 ){ // update LCD only if there is a change of rpm
    last_rpm = rpm;
    // need to check the case that there is 0 rpm (full stop)
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > refreshInterval) { // at a decent refresh rate
      previousMillis = currentMillis;   
      update_display();
    }
  } 
}

void update_display(){
  //Clear row 2
  lcd.backlight();
  lcd.setCursor(0, 1);
  lcd.print("                ");   
  //Update rpm on screen
  lcd.setCursor(0, 1);
  lcd.print(rpm);   
  //lcd.setCursor(7, 1);
  //lcd.print(time);   

}

void update_rpm(){
  //Update The RPM
  //5 sample moving average on rpm readings
  rpm_array[0] = rpm_array[1];
  rpm_array[1] = rpm_array[2];
  rpm_array[2] = rpm_array[3];
  rpm_array[3] = rpm_array[4];
  rpm_array[4] = 60*(1000000/(time*1)); // todo: handle case that there is full stop
  // for spindle a B/W marker is used and only 1 marker per rev
  // formula is explained here: http://www.pyroelectro.com/tutorials/tachometer_rpm_arduino/theory2.html
  //Last 5 Average RPM Counts Eqauls....
  rpm = (rpm_array[0] + rpm_array[1] + rpm_array[2] + rpm_array[3] + rpm_array[4]) / 5;
}

//Capture IR bounce-Beam via Interrupt, time marker on axis 1/1 ratio
void SENSOR_ISR()
{
  time = (micros() - time_last); 
  time_last = micros();
}





