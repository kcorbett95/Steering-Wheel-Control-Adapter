#include <SPI.h>
#define DATAOUT 11//COPI
#define DATAIN  12//CIPO
#define SPICLOCK  13//sck
#define CHIPSELECT 10//cs

//potentiometer selection values, do not alter
#define tip 0b00010001
#define ring 0b00010010
#define Both 0b00010011

//Definitions for output resistances required for stereo, these will need to be changed based on your head-unit and the
//resistance it requires for each media function. See ReadMe for more information
#define noBtn 100000
#define volUp 16000
#define volDn 24000
#define trackUp 8000
#define trackDn 11250
#define listInfo 5750
#define back 62750
#define muteEnter 3500
#define source 1200
#define voice 62750
#define offHook 5000
#define onHook 3500

// initialization code, do not alter
void setup() {
  Serial.begin(9600);
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(CHIPSELECT,OUTPUT);
  digitalWrite(CHIPSELECT,HIGH);
  SPI.begin();
  delay(100);
  potWrite(noBtn,Both);
}

//analog read to calculate resistance from vehicle button input, do not alter
double readRes(double analogPin) {
  
  double raw = 0;
  float vout = 0;
  float buffer = 0;
  double res = 0;
  delay(150);
  raw = analogRead(analogPin);
  if (raw>7) {
    buffer = raw * 5;
    vout = buffer / 1024;
    buffer = (5 / vout) - 1;
    res = 1000 * buffer;
    if(res <= 0){
      return noBtn;
    }
    // for debugging, uncomment to view input values in console
    // Serial.print("raw: ");
    // Serial.println(raw);
    // Serial.print("vout: ");
    // Serial.println(vout);
    // Serial.print("Res: ");
    // Serial.println(res);
    delay(50);
    return res;
  }
  else {
    return noBtn;
  }
}

//main function, see ReadMe for information on how to set input ranges and tip/ring output
void loop() {
  
    // Read the values of the two input wires from the steering wheel buttons
    double input1 = readRes(A0);
    double input2 = readRes(A1);

    // Map the values to the corresponding functions
    if (input1 < 25) {
      // Volume Up
      Serial.print("vol up\n");
      potWrite(volUp, tip);
      //delay(5000);
      while(readRes(A0) < 25){
        potWrite(volUp, tip);
        Serial.print("vol up hold\n");
      }
      potWrite(noBtn, tip);
      //delay(5000);
    }

    if (input1 >= 25 && input1 < 75) {
      // Volume Down
      Serial.print("vol dn\n");
      potWrite(volDn, tip);
      //delay(5000);
      while(readRes(A0) >= 25 && readRes(A0) < 75){
        Serial.print("vol dn hold\n");
        potWrite(volDn, tip);
      }
      potWrite(noBtn, tip);
    }

    if (input1 >= 75 && input1 < 175) {
      // Track +
      Serial.print("track up\n");
      potWrite(trackUp, tip);
      while(readRes(A0) >= 75 && readRes(A0) < 175){
        potWrite(trackUp, tip);
        Serial.print("track up hold\n");
      }
      //delay(5000);
      potWrite(noBtn, tip);
    }

    if (input1 >= 175 && input1 < 300) {
      // Track -
      Serial.print("track dn\n");
      potWrite(trackDn, tip);
      while(readRes(A0) >= 175 && readRes(A0) < 300){
        potWrite(trackDn, tip);
        Serial.print("track dn hold\n");
      }
      //delay(5000);
      potWrite(noBtn, tip);
    }

    if (input1 >= 300 && input1 < 575) {
      // List / Song info
      Serial.print("list/info\n");
      potWrite(listInfo, tip);
      while(readRes(A0) >= 300 && readRes(A0) < 575){
        potWrite(listInfo, tip);
        Serial.print("list info hold\n");
      }
      //delay(5000);
      potWrite(noBtn, tip);
    }

    if (input1 >= 575 && input1 < 1200) {
      // Back / Return
      Serial.print("return\n");
      potWrite(back, tip);
      while(readRes(A0) >= 575 && readRes(A0) < 1200){
        potWrite(back, tip);
        Serial.print("return hold\n");
      }
      //delay(5000);
      potWrite(noBtn, tip);
    }

    if (input2 < 25) {
      // Mute/Enter
      Serial.print("mute/enter\n");
      potWrite(muteEnter, tip);
      while(readRes(A1) < 25){
        potWrite(muteEnter, tip);
        Serial.print("mute enter hold\n");
      }
      //delay(5000);
      potWrite(noBtn, tip);
    }

    if (input2 >= 75 && input2 < 175) {
      // Source
      Serial.print("src\n");
      potWrite(source, tip);
      while(readRes(A1) >= 75 && readRes(A1) < 175){
        potWrite(source, tip);
        Serial.print("source hold\n");
      }
      //delay(5000);
      potWrite(noBtn, tip);
    }

    if (input2 >= 25 && input2 < 75) {
      // Voice Control
      Serial.print("voice\n");
      potWrite(voice, ring);
      delay(2000);  //delay required for some head units to register voice input
      while(readRes(A1) >= 25 && readRes(A1) < 75){
        potWrite(voice, ring);
        Serial.print("voice hold\n");
      }
      potWrite(noBtn, ring);
    }

    if (input2 >= 300 && input2 < 575) {
      // Off-Hook
      Serial.print("off hook\n");
      potWrite(offHook, ring);
      while(readRes(A1) >= 300 && readRes(A1) < 575){
        potWrite(offHook, ring);
        Serial.print("offHook hold\n");
      }
      //delay(5000);
      potWrite(noBtn, ring);
    }

    if (input2 >= 175 && input2 < 300) {
      // On-Hook
      Serial.print("on hook\n");
      potWrite(onHook, ring);
      while(readRes(A1) >= 175 && readRes(A1) < 300){
        potWrite(onHook, ring);
        Serial.print("onHook hold\n");
      }
      //delay(5000);
      potWrite(noBtn, ring);
    }
  }

//write function to set potentiometer output, do not alter
void potWrite(double res, int potNum) {
    //debugging
    //Serial.print("potres: ");
    //Serial.println(res);
    double resDec = (((res-52)*256)/noBtn);
    
    if(resDec >= noBtn){
      resDec = noBtn;
    }else if(resDec < 0){
      resDec = 0;
    }
    //debugging
    //Serial.print("resDec: ");
    //Serial.println(resDec);
    byte resByte = byte(resDec);
    if(resByte == 255){
      Serial.print("reset\n");
    }
    //debugging
    //Serial.print("resByte: ");
    //Serial.println(resByte);
    //byte testData = 0b00011001;
    digitalWrite(CHIPSELECT,LOW);
    SPI.transfer(potNum);
    SPI.transfer(resByte);
    digitalWrite(CHIPSELECT,HIGH);
    delay(250); 
  return;
}
