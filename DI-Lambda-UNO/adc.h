

#include <MCP342X.h>

MCP342X myADC;


#define LED_PIN 13

void led(bool status)
{
  if (status) {
    digitalWrite(LED_PIN, HIGH);
    //analogWrite(LED_PIN, 255); // Set 50% duty cycle
  } else {
    digitalWrite(LED_PIN, LOW); // Turn off
  }

  /* It takes the signal about 0.1ms to settle. */
  delay(1);
}

void adc_start(void) {
  
  Serial.println(myADC.testConnection() ? "MCP342X connection successful" : "MCP342X connection failed");

  myADC.configure( MCP342X_MODE_ONESHOT |
                   MCP342X_CHANNEL_1 |
                   MCP342X_SIZE_18BIT |
                   MCP342X_GAIN_1X
                 );

  pinMode(LED_PIN, OUTPUT);

}

void adc_sample(void callback(int32_t)) {
  int32_t signal = 0;
  int32_t bg = 0;
  uint32_t tmp;

  /* Get the reading from the sensor with the LED on. Then get the reading with LED off.
     This helps with rejecting backround light. */
  led(1);
  myADC.startConversion();
  while(myADC.checkforResult(&signal) & MCP342X_RDY)
    yield();
  
  led(0);
  myADC.startConversion();
  while(myADC.checkforResult(&bg) & MCP342X_RDY)
    yield();


  Serial.print("J: { \"bg\" : [ ");
  Serial.print(bg);
  Serial.print(", 0, 0");

  Serial.print("], \"signal\" : [ ");

  Serial.print(signal);
  Serial.print(", 0, 0");
  Serial.print("] }\r\n");

  callback(signal - bg);

  yield();
  //display_data(465, signal - bg, 10000, TRANSMITTANCE);
  
}
