/*
  Arduino EEPROM Read/Write via ICSP
  Author: Jon Raymond
  Date: March 19th 2017
  Version: 0.33

  The point of this program is to enable read/write/list of EEPROM values of a target
  AVR without interfacing with the code running on it. Handy if you need to modify
  parameters of chips running legacy code that you don't have source to.

  ------------------------------------------------------------
  "THE BEERWARE LICENSE":
  Jon Raymond pieced together this abomination of code. As long as you retain this
  notice, you can do whatever you want with this stuff. If we
  meet someday, and anything here helped you, you can
  buy me a beer in return. Cheers
  ------------------------------------------------------------
   Program based off work from the below sources

    Arduino EEPROM Read/Write using Serial Monitor
    https://circuits.io/circuits/1366613-arduino-eeprom-read-write-using-serial-monitor
    Author: Eric W. Johnson
    Date 13th December 2015

    ArduinoISP version 04m3
    https://www.arduino.cc/en/Tutorial/ArduinoISP
    Copyright (c) 2008-2011 Randall Bohn

    Atmega Board Detector
    https://github.com/nickgammon/arduino_sketches/tree/master/Atmega_Board_Detector
    Author: Nick Gammon
    Date: 22nd May 2012
    Version: 1.19

    AVR-standalone-ISP
    https://github.com/DeqingSun/AVR-standalone-ISP
    Author: DeqingSun
    Date: Mar 15, 2015

    http://www.atmel.com/Images/Atmel-2486-8-bit-AVR-microcontroller-ATmega8_L_datasheet.pdf
    Page 233 Serial Programming Set

*/


//#include <Time.h>
//#include <DS1307RTC.h>
#include "BurnEEPROM.h"
#include "readEEPROM.h"

// ********************************
// Declare and initialize variables
String strCommand = "";           // string variable to hold the command
char chrInput[128];               // character variable for reading bytes
boolean bDisplayMessage = true;   // boolean variable to determine whether or not to display message.
int iLocRead = 0;                 // integer variable for location of 'read'
int iLocWrite = 0;                // integer variable for location of 'write'
int iLocList = 0;                 // integer variable for location of 'list'
int iLocBurn = 0;                 // integer for burn
int iLocTest = 0;                 // integer for test
int iLocSpace = 0;                // integer variable for location of space
int iAddress = 0;                 // integer variable for EEPROM address
byte bytValue = 0;                // byte variable for value stored at EEPROM address
String strTmp = "";               // temporary string variable
//int pmode=0;

#define RESET 10  // Important - Reset of Target need to be connected to D10 of Master

//the five other pins of the ICSP header is used.



// Define data to be burned into EEPROM

long lowID = 17895697; //lower bound for 8 digit HEX SN.
long ID = 268435455; //upper bound for 8 digit HEX SN.
long Sint; // Serial number
String SN;
const String BF = "Temperature hubard16"; // Board Family
const String BV = "1.0"; // Board Version
const String CD =  __DATE__ " " __TIME__; // Compile Date


//build combined string of these data.

String Data = SN + ";" + BF + ";" + BV + ";" + CD;

// ********************************
// SETUP serial monitor
void setup() {
  //Serial.begin(9600);           // Set baud rate for serial monitor
  Serial.begin(115200);
}

// ********************************
void loop() {

  // Display informational message if it's not already displayed
  if (bDisplayMessage) {
    Serial.println(F("\nPlease enter a command. (L)ist, (B)urn (for burning date and serial), (T)est (for reading stored strings)"));
    bDisplayMessage = false;  // suppress message, for now.
  }

  // Get command and store in a string
  if (Serial.available()) {
    //Wait a bit, then read the serial buffer
    delay(100);

    if (false) {
      Serial.readBytes(chrInput, 128);
      strCommand = String(chrInput);
    }
    else {
      strCommand = Serial.readString();
      strCommand.toUpperCase();          // change to upper case to allow mixed case commands
    }

    // Determine if it's a read, write or list command
    iLocRead = strCommand.indexOf("READ");      // location of 'read'
    iLocWrite = strCommand.indexOf("WRITE");    // location of 'write'
    iLocList = strCommand.indexOf("L");    // location of 'list'
    iLocBurn = strCommand.indexOf("B");
    iLocTest = strCommand.indexOf("T");


    // Display error message if command not found.
    if (iLocRead < 0 && iLocWrite < 0 && iLocList < 0 && iLocBurn < 0 && iLocTest < 0 ) {
      Serial.println(F("\nNo read, write or list command found, please try again."));
    }
    else {

      if (iLocList >= 0) { // List locations and values for a set number
        PrintList();
      }


      else if (iLocBurn >= 0) {
        randomSeed(analogRead(0)*millis());
        Sint = random(lowID,ID); //lower bound for 8 character HEX SN. 
        SN = String(Sint,HEX);
        Data = SN + ";" + BF + ";" + BV + ";" + CD;
        BurnEEPROM(Data);
        
      }


      else if (iLocTest >= 0) {
        TestEEPROM(Data);
 
        
      }

    

    }   // END else{ Process Command

    bDisplayMessage = true;       // Get next command

  }   // END if (Serial.available())

}   // END void loop()



void PrintList(){
        Serial.println(F("\nList Target EEPROM"));
        Serial.print(F("Address"));
        Serial.print("\t");
        Serial.print(F("Value"));
        Serial.println();
        iAddress = 0;
        startProgramming();
        while (iAddress < 41) {
          bytValue = spi_transaction(0xA0, (iAddress >> 8) & 0x1F, iAddress & 0xFF, 0x00) & 0xFF;
          Serial.print(iAddress);
          Serial.print("\t");
          Serial.print(bytValue, DEC);
          Serial.println();
          iAddress++;
        }
        stopProgramming();
}
