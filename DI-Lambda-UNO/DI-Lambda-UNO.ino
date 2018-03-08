/*
     DI-Lambda UNO firmware
     Copyright (C) 2017  Alexey Zaytsev

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <Wire.h>

#include <Scheduler.h>

#define BTN_SAMPLE A0
#define BTN_BLANK A1
#define BTN_MODE A2


/*
#define BTN_SAMPLE PB4
#define BTN_BLANK PB4
#define BTN_MODE PB4
*/

/* A few global variables */
int32_t last_sample = -1;
int32_t blank = -1;
int32_t led_wl = 475;

void adc_callback(void);
void button_callback(void);


#include "lcd.h"
#include "adc.h"

void new_sample(int32_t sample)
{
  last_sample = sample;

  lcd_new_data();
}

void adc_callback(void)
{
  adc_sample(&new_sample);
}

void button_callback(void)
{
  //Serial.println("B");

  if (!digitalRead(BTN_BLANK)) {
    Serial.println("BLANK");
    blank = last_sample;
    lcd_new_data();
    delay(200);
    while (!digitalRead(BTN_BLANK)) {
      yield();
    }
  }

  if (!digitalRead(BTN_MODE)) {
    Serial.println("MODE");

    if (mode == MODE_LAST) {
      mode = MODE_FIRST;
    } else {
      mode = (display_mode_t) ((int)mode + 1);
    }
    display_mode(mode);
    delay(200);

    while (!digitalRead(BTN_MODE)) {
      yield();
    }
  }

  if (!digitalRead(BTN_SAMPLE)) {
    Serial.println("SAMPLE");
    delay(200);

    while (!digitalRead(BTN_SAMPLE)) {
      yield();
    }
  }
  yield();
}

void buttons_setup()
{
  pinMode(BTN_SAMPLE, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_BLANK, INPUT_PULLUP);
}

void setup(void)
{
  Serial.begin(115200);
                                                                                        
  Wire.begin();

  adc_start();

  buttons_setup();

  display_start();

  delay(500);
  display_clear();

  Scheduler.start(NULL, adc_callback);
  Scheduler.start(NULL, button_callback);

  mode = MODE_ABS;
  display_mode(mode);

  Serial.println("Setup done!");
}

void loop(void)
{
  yield();
}

