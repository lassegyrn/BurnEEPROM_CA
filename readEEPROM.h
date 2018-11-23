
#include <Arduino.h>
#include <EEPROM.h>

int address = 0;
byte value;
int Length = 50;


void LocalEEPROM(int DataLength) { // Function to read data local EEPROM
  Serial.println("Reading local EEPROM");

  uint8_t localValue[DataLength]; // constructs byte array length of string

  for (int i = 0; i <= DataLength + 1; i++) {
   
    localValue[i] =  EEPROM.read(i);
  }
  String test = String((char*)localValue);
  Serial.println(test);
  return test;
}
