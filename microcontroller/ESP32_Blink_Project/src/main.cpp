#include <Arduino.h> //this tells it to use all the standard arduino libraries

//This sets the LED we're using to the one on the device
#define LED 8

//this is the bits that run once when the device boots
void setup() {
// This sets the serial port speed (important for later)
Serial.begin(115200);

//This marks the LED PIN as an output device, this means we can send commands to it
pinMode(LED, OUTPUT);
}

//this is the bits that run repeatedly
void loop() {

//Turn the LED on
digitalWrite(LED, HIGH);

//print a message to the console
Serial.println("LED is on");
//wait one second, this is in milliseconds
delay(1000);
//Turn the LED off
digitalWrite(LED, LOW);
//print a message to the console
Serial.println("LED is off");
//wait one second, this is in milliseconds
delay(1000);
}