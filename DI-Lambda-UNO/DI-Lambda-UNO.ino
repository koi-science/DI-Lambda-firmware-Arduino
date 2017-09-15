/*
 *   DI-Lambda UNO firmware
 *   Copyright (C) 2017  Alexey Zaytsev
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <Wire.h>

#include <Adafruit_TSL2561_U.h>
#include <Adafruit_Sensor.h>


// The LED is powered from a 12v boost converter, to get stable voltage, and be
// able to drive LEDs that require more than 4.5v USB is guaranteed to provide.

#define BOOST_PIN 7 // Turn on the boost converter.


/* Supposedly, LEDs have narrower bandwidth when driven with higher current
 * in short pulses. This needs to be verified. */
#define LED_PIN 9   // Turn on the LED. Drive the LED at 50% duty cycle with PWM!

void led(bool status)
{
  if (status) {
    analogWrite(LED_PIN, 127); // Set 50% duty cycle
  } else {
    digitalWrite(LED_PIN, LOW); // Turn off
  }
}

/* We have 1 sensor on the UNO */
Adafruit_TSL2561_Unified tsl1 = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 12345);

void configureSensors(tsl2561IntegrationTime_t integration_time)
{
  tsl1.setGain(TSL2561_GAIN_16X);
  tsl1.setIntegrationTime(integration_time);
}

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(9600);
  Wire.begin();

  delay(50);

  Serial.println("Light Sensor Test...");

  /* Initialise the sensor */
  if (!tsl1.begin())
  {
    Serial.println("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");

    /* Get some more information to help to debug the probem */
    uint8_t id;
    Wire.beginTransmission(TSL2561_ADDR_LOW);
    Wire.requestFrom(0x86, 1);
    id = Wire.read();
    Wire.endTransmission();

    Serial.print("sensor ID=");
    Serial.println(id, HEX);
  } else {
    Serial.println("Sensor 1 OK...");
  }

  configureSensors(TSL2561_INTEGRATIONTIME_402MS);

  pinMode(LED_PIN, OUTPUT);

  pinMode(BOOST_PIN, OUTPUT);
  digitalWrite(BOOST_PIN, HIGH);

}

void loop(void)
{
  uint16_t signal;
  uint16_t bg;
  uint16_t tmp;

  /* Get the reading from the sensor with the LED on. Then get the reading with LED off.
   * This helps with rejecting backround light. */

  led(1);

  tsl1.enable(); /* Start the light sensor  */
  delay(420); /* Wait for the integration to finish. */
  tsl1.getLuminosity(&signal, &tmp);
  tsl1.disable();

  led(0);

  tsl1.enable();
  delay(420);
  tsl1.getLuminosity(&bg, &tmp);
  tsl1.disable();

  /* Output a json string. The desktop software expects 3 channels,
   * and we have 1 on UNO, so output zeros for channel 2 and 3.*/

  Serial.print("J: { \"bg\" : [ ");
  Serial.print(bg);
  Serial.print(", 0, 0");

  Serial.print("], \"signal\" : [ ");

  Serial.print(signal);
  Serial.print(", 0, 0");
  Serial.print("] }\r\n");
}

