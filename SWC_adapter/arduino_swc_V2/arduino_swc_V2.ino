#include <SPI.h>
#include <AceButton.h>
using namespace ace_button;

// Set the SWC input PINs
static const uint8_t BUTTON_PIN1 = A0;
static const uint8_t BUTTON_PIN2 = A1;

// Set the digipot wiper
#define DATAOUT 11     //COPI
#define DATAIN 12      //CIPO
#define SPICLOCK 13    //sck
#define CHIPSELECT 10  //cs

// Set the wiper addresses for the digipot
int wiperTip = B00010001;
int wiperRing = B00010010;

// Set the digitpot resistor value for no button
int ground = 0;
int noBtn = 255;

// Create AceButton objects, with their virtual pin number.
// Each virtual PIN relates to a resistance value on the ladder
static const uint8_t NUM_BUTTONS_1 = 6;
static AceButton volUp((uint8_t)0);
static AceButton volDn(1);
static AceButton trackUp(2);
static AceButton trackDn(3);
static AceButton listInfo(4);
static AceButton back(5);
//static AceButton open1(11);
static AceButton* const BUTTONS_1[NUM_BUTTONS_1] = {
  &volUp,
  &volDn,
  &trackUp,
  &trackDn,
  &listInfo,
  &back,
//  &open1,
};

static const uint8_t NUM_BUTTONS_2 = 5;
static AceButton muteEnter(6);
static AceButton source(7);
static AceButton voice(8);
static AceButton offHook(9);
static AceButton onHook(10);
static AceButton open2(12);
static AceButton* const BUTTONS_2[NUM_BUTTONS_2] = {
  &muteEnter,
  &source,
  &voice,
  &offHook,
  &onHook,
//  &open2,
};
//
// Define the ADC voltage levels for input pin 1 buttons.
// These values are read from SWC buttons
static const uint8_t NUM_LEVELS_1 = 7;
static const uint16_t LEVELS_1[NUM_LEVELS_1] = {
  1023 /* VOL+ */,
  1015 /* VOL- */,
  950 /* Track + */,
  860 /* Track - */,
  740 /* List/Info */,
  530 /* Return */,
  0 /* Open */,
};

// Define the ADC voltage levels for input pin 2 buttons.
// These values are read from SWC buttons
static const uint8_t NUM_LEVELS_2 = 6;
static const uint16_t LEVELS_2[NUM_LEVELS_2] = {
  1023 /* Mute/Enter */,
  950 /* SOURCE */,
  1015 /* Voice */,
  860 /* Off-Hook */,
  740 /* On-Hook */,
  0 /* Open */,
};

// The LadderButtonConfig constructor binds the AceButton to the
// LadderButtonConfig.
static LadderButtonConfig buttonConfig_1(
  BUTTON_PIN1, NUM_LEVELS_1, LEVELS_1, NUM_BUTTONS_1, BUTTONS_1, 0);

static LadderButtonConfig buttonConfig_2(
  BUTTON_PIN2, NUM_LEVELS_2, LEVELS_2, NUM_BUTTONS_2, BUTTONS_2, 0);

void handleEvent(AceButton*, uint8_t, uint8_t);

void setup() {
  //delay(1000);  // some microcontrollers reboot twice
  Serial.begin(9600);
  while (!Serial)
    ;  // Wait until Serial is ready - Leonardo/Micro
  Serial.println("setup(): begin");

  // Don't use internal pull-up resistor
  pinMode(BUTTON_PIN1, INPUT);
  pinMode(BUTTON_PIN2, INPUT);

  // Configure the ButtonConfig with the event handler.
  buttonConfig_1.setEventHandler(handleEvent);
  buttonConfig_1.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig_1.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig_1.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig_1.setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig_1.setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig_1.setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  buttonConfig_1.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  buttonConfig_2.setEventHandler(handleEvent);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  //  Ready digitpot, set wiper to no button
  pinMode(CHIPSELECT, OUTPUT);
  SPI.begin();
  digitalWrite(CHIPSELECT, LOW);
  SPI.transfer(wiperTip);   // command
  SPI.transfer(ground);       // value
  SPI.transfer(wiperRing);  // command
  SPI.transfer(noBtn);       // float the ring circuit
  digitalWrite(CHIPSELECT, HIGH);
  Serial.println("setup(): ready");
}

void loop() {
  // Should be called every 4-5ms or faster, for the default debouncing time
  // of ~20ms.
  buttonConfig_1.setDebounceDelay(50);
  buttonConfig_2.setDebounceDelay(50);
  buttonConfig_1.checkButtons();
  buttonConfig_2.checkButtons();
}

// In the function for the tip commands we only change resistance for the
// wiper where the tip is connected. The ring floats and is ignored by the
// head unit
void wrTip(int digiValue, int delayMs) {
  digitalWrite(CHIPSELECT, LOW);
  SPI.transfer(wiperTip);
  SPI.transfer(digiValue);
  Serial.print(" Button Press");  // for debug
  delay(delayMs);
  SPI.transfer(wiperTip);
  SPI.transfer(ground);
  Serial.print("\n Button Release TIP");  // for debug
  digitalWrite(CHIPSELECT, HIGH);
}

void wrTipHold(int digiValue) {
  digitalWrite(CHIPSELECT, LOW);
  SPI.transfer(wiperTip);
  SPI.transfer(digiValue);
  Serial.print("\n Hold TIP");  // for debug
  digitalWrite(CHIPSELECT, HIGH);
}

void wrTipRelease() {
  digitalWrite(CHIPSELECT, LOW);
  SPI.transfer(wiperTip);
  SPI.transfer(ground);
  Serial.print("\n Button Release TIP");  // for debug
  digitalWrite(CHIPSELECT, HIGH);
}

// For ring functions the wiper for the tip is where resistance is applied
// in combination with taking the ring to ground
void wrRing(int digiValue, int delayMs) {
  digitalWrite(CHIPSELECT, LOW);
  SPI.transfer(wiperRing);
  SPI.transfer(ground);
  Serial.print("\n Select Ring, short");  // for debug
  SPI.transfer(wiperTip);
  SPI.transfer(digiValue);
  Serial.print("\n Select Tip, button");  // for debug
  delay(delayMs);
  SPI.transfer(wiperTip);
  SPI.transfer(ground);
  SPI.transfer(wiperRing);
  SPI.transfer(noBtn);  // float
  Serial.print("\n Button Release Ring"); // for debug
  digitalWrite(CHIPSELECT, HIGH);
}

// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t /* buttonState */) {

  uint8_t swButton = button->getPin();
  uint8_t swPress = (eventType);

  
  // Print out a message for all analogue events for debug
  Serial.print("handleEvent(): ");
  Serial.print("virtualPin: ");
  Serial.println(swButton);
  Serial.print("; eventType: ");
  Serial.println(swPress);
 

  // Volume Up, Single Click
  if ((swButton == 0) && (swPress == 0)) {
    Serial.print("\n VOL UP press"); // for debug
    wrTip(42, 50);  // 24kOhm
  }

  // Volume Up, Hold
  if ((swButton == 0) && (swPress == 4)) {
    Serial.print("\n VOL Up Hold"); // for debug
    wrTipHold(42);  // 24kOhm
  }

  // Volume Up, Release Hold
  if ((swButton == 0) && (swPress == 6)) {
    Serial.print("\n VOL Up Release Hold"); // for debug
    wrTipRelease();
  }

  // Volume Down, Single Click
  // A pressed event is the most responsive for volume control
  if ((swButton == 1) && (swPress == 0)) {
    Serial.print("\n VOL DOWN");
    wrTip(55, 50);  // 16kOhm
  }

  // Volume Down, Hold
  if ((swButton == 1) && (swPress == 4)) {
    Serial.print("\n VOL DOWN Hold");
    wrTipHold(55);  // 16kOhm
  }

  // Volume Down, Release
  if ((swButton == 1) && (swPress == 6)) {
    Serial.print("\n VOL DOWN Release Hold");
    wrTipRelease();
  }

  /*
* Relase is suppressed and clicked is delayed, these events catch click and 
* double-click with a reasonale delay for the track & seek -/+ functions
* and source button functions. A slight delay in these functions is acceptable UX.
* At least, I think so anyway :)
*/

  // Next Track
  if ((swButton == 2) && (swPress == 0)) {
    Serial.print("\n Next Track");
    wrTip(19, 50);  // 8kOhm
  }

  // Previous Track
  if ((swButton == 3) && (swPress == 0)) {
    Serial.print("\n Previous Track");
    wrTip(27, 50);  // 11,25kOhm
  }

  //  List/Info
  if ((swButton == 4) && (swPress == 0)) {
    Serial.print("\n List Info");
    wrRing(161, 50);  // 62,75kOhm, try 14 tip for 5kohm if doesnt work
  }

  //  Return
  if ((swButton == 5) && (swPress == 0)) {
    Serial.print("\n Return");
    wrTip(161, 50);  // 62,75kOhm
  }

  //  Mute/Enter
  if ((swButton == 6) && (swPress == 0)) {
    Serial.print("\n Mute/Enter");
    wrTip(9, 50);  // 3.5k
  }

  //  Source
  if ((swButton == 7) && (swPress == 0)) {
    Serial.print("\n Source");
    wrTip(3, 50);  // 1.2kOhm
  }

  //  Voice Control
  if ((swButton == 8) && (swPress == 0)) {
    Serial.print("\n Voice Control");
    wrRing(18, 1000);  // 6.7k-88kOhm hold, try 200 ring if doesnt work
  }

  //  Off-Hook
  if ((swButton == 9) && (swPress == 0)) {
    Serial.print("\n Offr-Hook");
    wrRing(12, 50);  // 5kOhm
  }

  //  On-Hook
  if ((swButton == 10) && (swPress == 0)) {
    Serial.print("\n On-Hook");
    wrRing(7, 50);  // 3kOhm
  }
}
