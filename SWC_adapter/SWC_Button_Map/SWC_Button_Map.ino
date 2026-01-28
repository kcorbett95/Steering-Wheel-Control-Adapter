#include <SPI.h>
#include <AceButton.h>
#define DATAOUT 11     //COPI
#define DATAIN 12      //CIPO
#define SPICLOCK 13    //sck
#define CHIPSELECT 10  //cs

//potentiometer selection values, do not alter
#define tip 0b00010001
#define ring 0b00010010
#define potBoth 0b00010011

//Definitions for output resistances required for stereo, these will need to be changed based on your head-unit and the
//resistance it requires for each media function. See ReadMe for more information
#define noBtn 100000
#define volUp 18000
#define volDn 24000
#define trackUp 8500
#define trackDn 5000
#define listInfo 10000
#define back 80000
#define muteEnter 3500
#define source 500
#define voice 80000
#define offHook 5000
#define onHook 3500
#define short 0

// initialization code, do not alter
void setup() {
  Serial.begin(9600);
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK, OUTPUT);
  pinMode(CHIPSELECT, OUTPUT);

  SPI.begin();

}

//analog read to calculate resistance from vehicle button input, do not alter
double readRes(double analogPin) {

  double raw = 0;
  float vout = 0;
  float buffer = 0;
  double byteValue = 0;
   raw = analogRead(analogPin);
  if (raw > 10) {
    delay(20);
    buffer = raw;
    // for debugging, uncomment to view input values in console
    Serial.print("\nBuffer: ");
    Serial.println(buffer);
    // Serial.print("vout: ");
    // Serial.println(vout);
    // Serial.print("Res: ");
    // Serial.println(byteValue);
    //delay(50);
    return buffer;
  // } else {
  //   return noBtn;
  }
}

//main function, see ReadMe for information on how to set input ranges and tip/ring output
void loop() {

  // Read the values of the two input wires from the steePot1 wheel buttons
  double input1 = readRes(A0);
  double input2 = readRes(A1);
  Serial.print("\nA0 Raw: ");
  Serial.println(input1);
  Serial.print("\nA1 Raw: ");
  Serial.println(input2);

  return;
}