/*

  This code used to control the digital potentiometer
  MCP41100 connected to  arduino Board
  CS >>> D10
  SCLK >> D13
  DI  >>> D11
  PA0 TO VCC
  PBO TO GND
  SHDN >> 9
  PW0 TO led with resistor 100ohm .
*/
#include <SPI.h>
byte addressPot0 =     0b00010001;      //To define potentiometer use last two BITS 01= POT 0
byte addressPot1 =     0b00010010;      //To define potentiometer use last two BITS 10= POT 1
byte addressPot0and1 = 0b00010011;  //To define potentiometer use last two BITS 10= POT 0 and 1
byte CS= 10;    //Chip control goes to pin 10
byte SHDN = 9;  //Chip SHUTDOWN - PIN 9
byte RS = 8;    //Chip RESET - PIN 8

void setup()
{
  pinMode (CS, OUTPUT); //CS - When High, sets chip to read the data.
  pinMode (SHDN, OUTPUT); //CS - When High, sets chip to read the data.
  pinMode (RS, OUTPUT); //CS - When High, sets chip to read the data.
  
  digitalWrite(SHDN, HIGH); //Power ON (HIGH)
  digitalWrite(RS, HIGH); //Power NO RESET (LOW)
  SPI.begin();
}

void loop()
{
  PotHighAndLow_mt(addressPot0);      //Change POT values on Pot0
  PotHighAndLow_mt(addressPot1);     //Change POT values on Pot1
  PotHighAndLow_mt(addressPot0and1); //Change POT values on both
  
  
  digitalPotWrite(245,  addressPot1);
  digitalWrite(SHDN, LOW); //Power OFF (LOW)
  delay(5000); //delay for 5 seconds to test current consumption of potentiometer
  digitalWrite(SHDN, HIGH); //Power ON (HIGH)
  digitalPotWrite(245,  addressPot1);
  
  digitalWrite(RS, LOW); //Power NO RESET (LOW)
  delay(100); //delay 100 mls
  digitalWrite(RS, HIGH); //Power RESET (HIGH)
  delay(10000); //delay 10 sec

}

void PotHighAndLow_mt(byte address)
{
  /* We have limit from 130 - 255 just for LED test, but for other projects it can be 0-255 */
      for (int i = 130; i <= 255; i++)
    {
      digitalPotWrite(i,address);
      delay(10);
    }
   // delay(500);
    for (int i = 255; i >= 130; i--) 
    {
      digitalPotWrite(i,address);
      delay(10);
    }
}



int digitalPotWrite(byte value, byte address)
{
  digitalWrite(CS, LOW); //Set Chip Active
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(CS, HIGH); //Set Chip Inactive
}
