
/*
Author: AnalysIR
Revision: 1.0 - Initial release
Revision: 1.1 - update generic digitalPinToInterrupt to support most arduino platform

This code is provided to overcome an issue with Arduino IR libraries
It allows you to capture raw timings for signals longer than 255 marks & spaces.
Typical use case is for long Air conditioner signals.

You can use the output to plug back into IRremote, to resend the signal.

This Software was written by AnalysIR.

Usage: Free to use, subject to conditions posted on blog below.
Please credit AnalysIR and provide a link to our website/blog, where possible.

Copyright AnalysIR 2014-2019

Please refer to the blog posting for conditions associated with use.
http://www.analysir.com/blog/2014/03/19/air-conditioners-problems-recording-long-infrared-remote-control-signals-arduino/

Connections:
IR Receiver      Arduino
V+          ->  +5v
GND          ->  GND
Signal Out   ->  Digital Pin 2
(If using a 3V3 Arduino, you should connect V+ to +3V3)

Tested on UNO only

--
Modified by naya.vu
*/

//you may increase this value on Arduinos with greater than 2k SRAM
#define maxLen 500
#define rxPinIR 2 //pin D2 or D3 on standard arduinos. (other pins may be available on More mordern modules like MEga2560, DUE, ESP8266, ESP32)

volatile unsigned int irBuffer[maxLen]; //stores timings - volatile because changed by ISR
volatile unsigned int x = 0; //Pointer thru irBuffer - volatile because changed by ISR
volatile unsigned long lastTime = 0; //micros() of the previous IR impulse in a sequence of impulses - must be ulong

void setup() {
  Serial.begin(115200); //change BAUD rate as required
  attachInterrupt(digitalPinToInterrupt(rxPinIR), rxIR_Interrupt_Handler, CHANGE);//set up ISR for receiving IR signal
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println(F("Press a button on the controller"));
  delay(5000); // pause 5 secs
  if (x) { //if a signal is captured
    Serial.println();
    Serial.print(F("Raw ("));
    Serial.print((x - 1)); //dump raw header format - for library
    Serial.print(F("): "));
    detachInterrupt(digitalPinToInterrupt(rxPinIR));//stop interrupts & capture until finshed here

    // now dump the times
    for (int i = 0; i < x; i++) {
        Serial.print(irBuffer[i]);
        Serial.print(F(" "));
    }

    x = 0;
    Serial.println();
    Serial.println();
    attachInterrupt(digitalPinToInterrupt(rxPinIR), rxIR_Interrupt_Handler, CHANGE);//re-enable ISR for receiving IR signal
  }

}

void rxIR_Interrupt_Handler() {
  if (x > maxLen) {
    return; //ignore if irBuffer is already full
  }
  unsigned long currentTime = micros();
  irBuffer[x] = x == 0 ? 0 : currentTime - lastTime;
  lastTime = currentTime;
  x++;
}
