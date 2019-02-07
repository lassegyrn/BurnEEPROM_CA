
#include <Arduino.h>
#include <EEPROM.h>

#include <SPI.h>
#define RESET 10 

void spi_init() {
  uint8_t x;
  SPCR = 0x53;
  x = SPSR;
  x = SPDR;
}

void spi_wait() {
  do {
  }
  while (!(SPSR & (1 << SPIF)));
}

uint8_t spi_send(uint8_t b) {
  uint8_t reply;
  SPDR = b;
  spi_wait();
  reply = SPDR;
  return reply;
}

uint8_t spi_transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  uint8_t n;
  spi_send(a);
  n = spi_send(b);
  //if (n != a) error = -1;
  n = spi_send(c);
  return spi_send(d);
}




void startProgramming() {
  spi_init();
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, HIGH);
  pinMode(SCK, OUTPUT);
  digitalWrite(SCK, LOW);
  delay(50);
  digitalWrite(RESET, LOW);
  delay(50);
  pinMode(MISO, INPUT);
  pinMode(MOSI, OUTPUT);
  spi_transaction(0xAC, 0x53, 0x00, 0x00);
  //  pmode = 1
}

void stopProgramming() {
  digitalWrite (RESET, LOW);
  digitalWrite (SCK,   LOW);
  digitalWrite (MOSI,  LOW);
  digitalWrite (MISO,  LOW);
  pinMode (RESET, INPUT);
  pinMode(MISO, INPUT);
  pinMode(MOSI, INPUT);
  pinMode(SCK, INPUT);
  //  pmode = 0;
}

/* SPI functions  */



/* Custom functions for Copenhagen atomics */

void BurnEEPROM(String Data) { //Function to burn data to slave EEPROM, takes input string Data

  Serial.println("Trying to burn " + Data + " slave eeprom");

  uint8_t payload[Data.length() + 1]; //constructs bytearray with the length of the string
  Data.getBytes(payload, Data.length() + 1); // stores the string in the payload byte array

  startProgramming();

  for (int i = 0; i <= Data.length() + 1; i++) {

    spi_transaction(0xC0, (i >> 8) & 0xFF, i & 0xFF, payload[i]); // Write value to address

    //Serial.println("Burning " + String(payload[i - 1]) + " at location " + String(i) );

    //  Serial.println("byte no " + String(i) + " equals to "  +payload[i]);
    //  String test = String((char*)payload);
    //  Serial.println(test);
    delay(10);
    
  }
  Serial.println("Done, wrote " + Data + " to EEPROM");
  Serial.println("Verifying ...");
    uint8_t bytValue[Data.length()];
    for (int i = 0; i <= Data.length() + 1; i++) {
    // ** Need to add check if startProgramming was successfull as Read returns 255 if not connected
    bytValue[i] = spi_transaction(0xA0, (i >> 8) & 0x1F, i & 0xFF, 0x00) & 0xFF;
  }
  String test = String((char*)bytValue);
  if (test == Data){
    Serial.println("Complete and verified");
  }
  else{
    Serial.println("Not verified");
  }
  stopProgramming();
}


void TestEEPROM(String Data) { // Function to read data from slave EEPROM, takes string data to know expected size of data.
  Serial.println("Trying to read slave eeprom");

  uint8_t bytValue[Data.length()]; // constructs byte array length of string

  startProgramming();
  for (int i = 0; i <= Data.length() + 1; i++) {
    // ** Need to add check if startProgramming was successfull as Read returns 255 if not connected
    bytValue[i] = spi_transaction(0xA0, (i >> 8) & 0x1F, i & 0xFF, 0x00) & 0xFF;
  }
  String test = String((char*)bytValue);
  Serial.println(test);
  


  // for (int i=0; i <= sizeof(bytValue);i++){
  // Serial.println(bytValue[i]);
  // }

  stopProgramming();
}
