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
#define ground 0
#define volUp 42
#define volDn 55
#define trackUp 19
#define trackDn 27
#define listInfo 15
#define back 160
#define muteEnter 9
#define source 5
#define voice 200
#define offHook 12
#define onHook 7
#define open 100000
// initialization code, do not alter
void setup() {
  Serial.begin(9600);
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK, OUTPUT);
  pinMode(CHIPSELECT, OUTPUT);
  digitalWrite(CHIPSELECT, HIGH);
  SPI.begin();
  //delay(100);
  potWrite(ground, tip);
  potWrite(open, ring);
}

//analog read to calculate resistance from vehicle button input, do not alter
double readRes(double analogPin) {

  double raw = 0;
  float vout = 0;
  float buffer = 0;
  double writeValue = 0;

  raw = analogRead(analogPin);
  delay(20);
  if (raw < 100000) {

    // for debugging, uncomment to view input values in console
    // Serial.print("raw: ");
    // Serial.println(raw);
    // Serial.print("vout: ");
    // Serial.println(vout);
    // Serial.print("Res: ");
    // Serial.println(writeValue);
    //delay(50);
    return raw;
  } else {
    return ground;
  }
}

//main function, see ReadMe for information on how to set input ranges and tip/ring output
void loop() {
  potWrite(ground, tip);
  potWrite(open, ring);
  // Read the values of the two input wires from the steePot1 wheel buttons
  double input1 = readRes(A0);
  double input2 = readRes(A1);

  // Map the values to the corresponding functions
  if (input1 >= 1020) {
    // Volume Up
    Serial.print("vol up\n");
    potWrite(volUp, tip);
    //delay(5000);
    while (readRes(A0) >= 1020) {
      potWrite(volUp, tip);
      Serial.print("vol up hold\n");
    }
    potWrite(ground, tip);
    //delay(5000);
  }

  if (input1 >= 950 && input1 < 1020) {
    // Volume Down
    Serial.print("vol dn\n");
    potWrite(volDn, tip);
    //delay(5000);
    while (readRes(A0) >= 950 && readRes(A0) < 1020) {
      Serial.print("vol dn hold\n");
      potWrite(volDn, tip);
    }
    potWrite(ground, tip);
  }

  if (input1 >= 900 && input1 < 950) {
    // Track +
    Serial.print("track up\n");
    potWrite(trackUp, tip);
    while (readRes(A0) >= 900 && readRes(A0) < 950) {
      potWrite(trackUp, tip);
      Serial.print("track up hold\n");
    }
    //delay(5000);
    potWrite(ground, tip);
  }

  if (input1 >= 800 && input1 < 900) {
    // Track -
    Serial.print("track dn\n");
    potWrite(trackDn, tip);
    while (readRes(A0) >= 800 && readRes(A0) < 900) {
      potWrite(trackDn, tip);
      Serial.print("track dn hold\n");
    }
    //delay(5000);
    potWrite(ground, tip);
  }

  if (input1 >= 700 && input1 < 800) {
    // List / Song info
    Serial.print("list/info\n");
    potWrite(listInfo, tip);
    while (readRes(A0) >= 700 && readRes(A0) < 800) {
      potWrite(listInfo, tip);
      Serial.print("list info hold\n");
    }
    //delay(5000);
    potWrite(ground, tip);
    //potWrite(open, ring);
  }

  if (input1 >= 450 && input1 < 700) {
    // Back / Return
    Serial.print("return\n");
    potWrite(back, tip);
    while (readRes(A0) >= 450 && readRes(A0) < 700) {
      potWrite(back, tip);
      Serial.print("return hold\n");
    }
    //delay(5000);
    potWrite(ground, tip);
  }

  if (input2 >= 1020) {
    // Mute/Enter
    Serial.print("mute/enter\n");
    potWrite(muteEnter, tip);
    while (readRes(A1) >= 1020) {
      potWrite(muteEnter, tip);
      Serial.print("mute enter hold\n");
    }
    //delay(5000);
    potWrite(ground, tip);
    
  }

  if (input2 >= 900 && input2 < 950) {
    // Source
    Serial.print("src\n");
    potWrite(source, tip);
    while (readRes(A1) >= 900 && readRes(A1) < 950) {
      potWrite(source, tip);
      Serial.print("source hold\n");
    }
    //delay(5000);
    potWrite(ground, tip);
  }

  if (input2 >= 950 && input2 < 1020) {
    // Voice Control
    Serial.print("voice\n");
    ringWrite(voice);
    //delay(2000);  //delay required for some head units to register voice input
    while (readRes(A1) >= 950 && readRes(A1) < 1020) {
      ringWrite(voice);
      Serial.print("voice hold\n");
    }
    potWrite(open, ring);
    potWrite(ground, tip);
  }

  if (input2 >= 800 && input2 < 900) {
    // Off-Hook
    Serial.print("off hook\n");
    ringWrite(offHook);
    while (readRes(A1) >= 800 && readRes(A1) < 900) {
      ringWrite(offHook);
      Serial.print("offHook hold\n");
    }
    //delay(5000);
    potWrite(ground, tip);
    potWrite(open, ring);
  }

  if (input2 >= 600 && input2 < 800) {
    // On-Hook
    Serial.print("on hook\n");
    ringWrite(onHook);
    while (readRes(A1) >= 600 && readRes(A1) < 800) {
      ringWrite(onHook);
      Serial.print("onHook hold\n");
    }
    //delay(5000);
    potWrite(ground, tip);
    potWrite(open, ring);
  }
}

//write function to set potentiometer output, do not alter
void potWrite(double writeValue, int potNum) {

  if (writeValue < ground) {
    writeValue = ground;
  } else if (writeValue > open) {
    writeValue = open;
  }
  //debugging
  //Serial.print("writeValue: ");
  //Serial.println(writeValue);
  //byte testData = 0b00011001;
  digitalWrite(CHIPSELECT, LOW);
  SPI.transfer(potNum);
  SPI.transfer(writeValue);
  digitalWrite(CHIPSELECT, HIGH);
  delay(20);

  return;
}

//write function to set potentiometer output, do not alter
void ringWrite(double writeValue) {
  //debugging
  //Serial.print("\nRing writeValue: ");
  //Serial.println(writeValue);
  //byte testData = 0b00011001;
  //potWrite(ground, ring);
  digitalWrite(CHIPSELECT, LOW);
  SPI.transfer(ring);
  SPI.transfer(ground);
  //  Serial.println(" Select ring, ground");  // for debug
  digitalWrite(CHIPSELECT, HIGH);
  delay(50);
  //potWrite(writeValue, tip);
  digitalWrite(CHIPSELECT, LOW);
  SPI.transfer(tip);
  SPI.transfer(writeValue);
  digitalWrite(CHIPSELECT, HIGH);
  delay(50);
  // SPI.transfer(255);
  // // Serial.println(" Select ring, ground");  // for debug
  // SPI.transfer(tip);
  // SPI.transfer(255);
  // digitalWrite(CHIPSELECT, HIGH);

  return;
}