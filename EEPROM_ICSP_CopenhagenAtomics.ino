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


#include <Time.h>
#include <DS1307RTC.h>
#include "BurnEEPROM.h"

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

const String SN = "AD8Kr0ft"; // Serial number
const String BF = "Temperature 4xRJ45"; // Board Family
const String BV = "1.0001"; // Board Version
const String CD =  __DATE__ " " __TIME__; // Compile Date


//build combined string of these data.

const String Data = SN + ";" + BF + ";" + BV + ";" + CD;

// ********************************
// SETUP serial monitor
void setup() {
  Serial.begin(9600);           // Set baud rate for serial monitor
  //Serial.begin(115200);
}

// ********************************
void loop() {

  // Display informational message if it's not already displayed
  if (bDisplayMessage) {
    Serial.println(F("\nPlease enter a command. Read <address>, Write <address> <value> Or List, Burn (for burning date and serial), Test (for reading stored strings)"));
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
    iLocList = strCommand.indexOf("LIST");    // location of 'list'
    iLocBurn = strCommand.indexOf("BURN");
    iLocTest = strCommand.indexOf("TEST");


    // Display error message if command not found.
    if (iLocRead < 0 && iLocWrite < 0 && iLocList < 0 && iLocBurn < 0 && iLocTest < 0 ) {
      Serial.println(F("\nNo read, write or list command found, please try again."));
    }
    else {

      if (iLocRead >= 0) {    // Process read command
        strTmp = strCommand.substring(iLocRead + 5);
        iAddress = strTmp.toInt();
        startProgramming();
        // ** Need to add check if startProgramming was successfull as Read returns 255 if not connected
        bytValue = spi_transaction(0xA0, (iAddress >> 8) & 0x1F, iAddress & 0xFF, 0x00) & 0xFF;
        stopProgramming();
        Serial.println("The contents of Target EEPROM address " + String(iAddress) + " is " + String(bytValue));
      }
      else if (iLocList >= 0) { // List locations and values for a set number
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


      else if (iLocBurn >= 0) {

        BurnEEPROM(Data);

      }


      else if (iLocTest >= 0) {

        ReadEEPROM(Data);
      }

      else {                  // Process write command
        strTmp = strCommand.substring(iLocWrite + 6);
        iAddress = strTmp.toInt();
        iLocSpace = strTmp.indexOf(" ");        // location of space
        strTmp = strTmp.substring(iLocSpace + 1);
        bytValue = strTmp.toInt();
        startProgramming();
        spi_transaction(0xC0, (iAddress >> 8) & 0xFF, iAddress & 0xFF, bytValue); // Write value to address
        // ** Need to add check if value was written to address
        stopProgramming();
        Serial.println("The value of " + String(bytValue) + " has been saved to Targets EEPROM address " + String(iAddress));
      }

    }   // END else{ Process Command

    bDisplayMessage = true;       // Get next command

  }   // END if (Serial.available())

}   // END void loop()



//void startProgramming() {
//  spi_init();
//  pinMode(RESET, OUTPUT);
//  digitalWrite(RESET, HIGH);
//  pinMode(SCK, OUTPUT);
//  digitalWrite(SCK, LOW);
//  delay(50);
//  digitalWrite(RESET, LOW);
//  delay(50);
//  pinMode(MISO, INPUT);
//  pinMode(MOSI, OUTPUT);
//  spi_transaction(0xAC, 0x53, 0x00, 0x00);
//  //  pmode = 1
//}
//
//void stopProgramming() {
//  digitalWrite (RESET, LOW);
//  digitalWrite (SCK,   LOW);
//  digitalWrite (MOSI,  LOW);
//  digitalWrite (MISO,  LOW);
//  pinMode (RESET, INPUT);
//  pinMode(MISO, INPUT);
//  pinMode(MOSI, INPUT);
//  pinMode(SCK, INPUT);
//  //  pmode = 0;
//}
//
///* SPI functions  */
//
//void spi_init() {
//  uint8_t x;
//  SPCR = 0x53;
//  x = SPSR;
//  x = SPDR;
//}
//
//void spi_wait() {
//  do {
//  }
//  while (!(SPSR & (1 << SPIF)));
//}
//
//uint8_t spi_send(uint8_t b) {
//  uint8_t reply;
//  SPDR = b;
//  spi_wait();
//  reply = SPDR;
//  return reply;
//}
//
//uint8_t spi_transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
//  uint8_t n;
//  spi_send(a);
//  n = spi_send(b);
//  //if (n != a) error = -1;
//  n = spi_send(c);
//  return spi_send(d);
//}



//
///* Custom functions for Copenhagen atomics */
//
//void BurnEEPROM(String Data) { //Function to burn data to slave EEPROM, takes input string Data
//
//  Serial.println("Trying to burn " + String(SN) + " slave eeprom");
//
//  uint8_t payload[Data.length() + 1]; //constructs bytearray with the length of the string
//  Data.getBytes(payload, Data.length() + 1); // stores the string in the payload byte array
//
//  startProgramming();
//
//  for (int i = 0; i <= Data.length() + 1; i++) {
//
//    spi_transaction(0xC0, (i >> 8) & 0xFF, i & 0xFF, payload[i]); // Write value to address
//
//    Serial.println("Burning " + String(payload[i - 1]) + " at location " + String(i) );
//
//    //  Serial.println("byte no " + String(i) + " equals to "  +payload[i]);
//    //  String test = String((char*)payload);
//    //  Serial.println(test);
//    delay(10);
//  }
//  stopProgramming();
//  return;
//}
//
//
//void ReadEEPROM(String Data) { // Function to read data from slave EEPROM, takes string data to know expected size of data.
//  Serial.println("Trying to read slave eeprom");
//
//  uint8_t bytValue[Data.length()]; // constructs byte array length of string
//
//  startProgramming();
//  for (int i = 0; i <= Data.length() + 1; i++) {
//    // ** Need to add check if startProgramming was successfull as Read returns 255 if not connected
//    bytValue[i] = spi_transaction(0xA0, (i >> 8) & 0x1F, i & 0xFF, 0x00) & 0xFF;
//  }
//  String test = String((char*)bytValue);
//  Serial.println(test);
//
//
//
//  // for (int i=0; i <= sizeof(bytValue);i++){
//  // Serial.println(bytValue[i]);
//  // }
//
//  stopProgramming();
//  return;
//}
