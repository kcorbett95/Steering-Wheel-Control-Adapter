#include <AceButton.h>
using namespace ace_button;

// Set the SWC input PINs
static const uint8_t BUTTON_PIN1 = A0;
static const uint8_t BUTTON_PIN2 = A1;

// NEC output pin that drives the transistor base
static const uint8_t NEC_PIN = 13;

// Kenwood NEC parameters
static const uint8_t NEC_ADDRESS = 0xB9;
static const uint16_t NEC_BASE = 562;         // us
static const uint16_t NEC_HDR_MARK = 9000;    // 16 * 562.5
static const uint16_t NEC_HDR_SPACE = 4500;   // 8 * 562.5
static const uint16_t NEC_BIT_MARK = 562;
static const uint16_t NEC_ONE_SPACE = 1687;   // 3 * 562.5
static const uint16_t NEC_ZERO_SPACE = 562;
static const uint16_t NEC_RPT_SPACE = 2250;   // 4 * 562.5
static const uint16_t NEC_REPEAT_INTERVAL_MS = 95;

// Event bookkeeping for NEC repeat frames
static uint8_t lastCommand = 0xFF;
static bool repeatActive = false;
static uint32_t lastRepeatMillis = 0;

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
  &volUp,     // 0
  &volDn,     // 48
  &trackUp,   // 111
  &trackDn,   // 197
  &listInfo,  // 319
  &back,      // 530
  //&open1,     // 1023
};

// Virtual pins 6-10 map low->high thresholds: onHook, offHook, source, voice, mute
static const uint8_t NUM_BUTTONS_2 = 5;
static AceButton muteEnter((uint8_t)0);
static AceButton voice(1);
static AceButton source(2);
static AceButton offHook(3);
static AceButton onHook(4);
//static AceButton open2(12);
static AceButton* const BUTTONS_2[NUM_BUTTONS_2] = {
  &muteEnter, // 0
  &voice,     // 47
  &source,    // 108
  &offHook,   // 195
  &onHook,    // 317
  //&open2,     // 1023
};

// Map virtual button index -> Kenwood NEC command (LSB first). Update as needed.
// Unused entries should be 0xFF.
//0xFC,   //AM tuner
//0x83,   //power off

static const uint8_t NEC_COMMANDS_1[NUM_BUTTONS_1] = {
  0x14,  // vol+
  0x15,  // vol-
  0x0B,  // track +
  0x0A,  // track -
  0x8A,  // list/info -> phone menu
  0x0E,  // back (play/pause)
};

static const uint8_t NEC_COMMANDS_2[NUM_BUTTONS_2] = {
  0x16,  // mute/enter
  0x8B,  // voice
  0x13,  // source
  0x88,  // off-hook
  0x89,  // on-hook
};
static const uint8_t NEC_CMD_PLAY_PAUSE = 0x0E;

//
// Define the ADC voltage levels for input pin 1 buttons (ascending, one per button),
// plus the open (no-button) level as the last entry.
static const uint8_t NUM_LEVELS_1 = NUM_BUTTONS_1 + 1;
static const uint16_t LEVELS_1[NUM_LEVELS_1] = {
  0     /* VOL+ */,
  47   /* VOL- */,
  111   /* Track + */,
  196   /* Track - */,
  319   /* List/Info */,
  529  /* Return/back */,
  1023  /* Open */,
};

// Define the ADC voltage levels for input pin 2 buttons (ascending, one per button),
// plus the open (no-button) level as the last entry.
static const uint8_t NUM_LEVELS_2 = NUM_BUTTONS_2 + 1;
static const uint16_t LEVELS_2[NUM_LEVELS_2] = {
  0     /* Mute/Enter */,
  48   /* Voice */,
  111   /* SOURCE */,
  195   /* Off-Hook */,
  319  /* On-Hook */,
  1023  /* Open */,
};

// The LadderButtonConfig constructor binds the AceButton to the
// LadderButtonConfig.
static LadderButtonConfig buttonConfig(
  BUTTON_PIN1, NUM_LEVELS_1, LEVELS_1, NUM_BUTTONS_1, BUTTONS_1, 1);

static LadderButtonConfig buttonConfig_2(
  BUTTON_PIN2, NUM_LEVELS_2, LEVELS_2, NUM_BUTTONS_2, BUTTONS_2, 1);

void handleEventA0(AceButton*, uint8_t, uint8_t);
void handleEventA1(AceButton*, uint8_t, uint8_t);
void handleEventWithMap(AceButton*, uint8_t, const uint8_t*, uint8_t, const __FlashStringHelper*, bool);
void sendKenwood(uint8_t cmd);
void sendKenwoodRepeat();
void handleSerialCommand();
void necMark(uint16_t usec);
void necSpace(uint16_t usec);
void necSendByte(uint8_t data);
void necSendFrame(uint8_t addr, uint8_t cmd);

void setup() {
  delay(1000);  // some microcontrollers reboot twice
  Serial.begin(115200);
  // Wait briefly for Serial on native-USB boards; don't block forever in vehicle power.
  unsigned long serialStart = millis();
  while (!Serial && (millis() - serialStart < 2000)) {
  }
  Serial.println(F("setup(): begin"));

  // Don't use internal pull-up resistor
  pinMode(BUTTON_PIN1, INPUT);
  pinMode(BUTTON_PIN2, INPUT);

  // NEC output defaults low; transistor pulls line when high.
  pinMode(NEC_PIN, OUTPUT);
  digitalWrite(NEC_PIN, LOW);

  // Configure the ButtonConfig with the event handler.
  buttonConfig.setEventHandler(handleEventA0);
  buttonConfig.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  buttonConfig_2.setEventHandler(handleEventA1);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  buttonConfig_2.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  
  
  Serial.println(F("setup(): ready"));
}

void loop() {
  // Should be called every 4-5ms or faster, for the default debouncing time
  // of ~20ms.
  buttonConfig.setDebounceDelay(80);
  buttonConfig_2.setDebounceDelay(80);
  buttonConfig.checkButtons();
  buttonConfig_2.checkButtons();
  if (repeatActive && lastCommand != 0xFF) {
    uint32_t now = millis();
    if (now - lastRepeatMillis >= NEC_REPEAT_INTERVAL_MS) {
      sendKenwood(lastCommand);
      lastRepeatMillis = now;
    }
  }
  handleSerialCommand();
}

// The event handler for the button.
void handleEventA0(AceButton* button, uint8_t eventType, uint8_t /* buttonState */) {
  handleEventWithMap(button, eventType, NEC_COMMANDS_1, NUM_BUTTONS_1, F("A0"), true);
}

void handleEventA1(AceButton* button, uint8_t eventType, uint8_t /* buttonState */) {
  handleEventWithMap(button, eventType, NEC_COMMANDS_2, NUM_BUTTONS_2, F("A1"), false);
}

void handleEventWithMap(
    AceButton* button,
    uint8_t eventType,
    const uint8_t* commands,
    uint8_t numButtons,
  const __FlashStringHelper* label,
  bool allowRepeat) {
  uint8_t swButton = button->getPin();
  uint8_t swPress = (eventType);

  if (swButton >= numButtons) {
    return;
  }

  Serial.print(F("handleEvent("));
  Serial.print(label);
  Serial.print(F("): virtualPin: "));
  Serial.print(swButton);
  Serial.print(F("; eventType: "));
  Serial.print(swPress);
  Serial.print(F("; A0: "));
  Serial.print(analogRead(BUTTON_PIN1));
  Serial.print(F("; A1: "));
  Serial.println(analogRead(BUTTON_PIN2));

  uint8_t cmd = commands[swButton];
  if (cmd == 0xFF) {
    return;
  }

  bool isRepeatButton = allowRepeat && (swButton == 0 || swButton == 1);

  switch (swPress) {
    case AceButton::kEventPressed:  // send only on initial press to avoid duplicate click events on release
      lastCommand = cmd;
      repeatActive = false;
      sendKenwood(cmd);
      break;

    case AceButton::kEventLongPressed:
    case AceButton::kEventRepeatPressed:
      if (!allowRepeat && swButton == 0 && swPress == AceButton::kEventLongPressed) {
        lastCommand = NEC_CMD_PLAY_PAUSE;
        repeatActive = false;
        sendKenwood(NEC_CMD_PLAY_PAUSE);
        break;
      }
      if (isRepeatButton) {
        repeatActive = true;
        lastRepeatMillis = 0;
      }
      break;

    case AceButton::kEventReleased:
    case AceButton::kEventLongReleased:
      lastCommand = 0xFF;
      repeatActive = false;
      break;

    default:
      break;
  }
}


void sendKenwood(uint8_t cmd) {
  necSendFrame(NEC_ADDRESS, cmd);
}

void sendKenwoodRepeat() {
  if (lastCommand == 0xFF) {
    return;
  }
  noInterrupts();
  necMark(NEC_HDR_MARK);
  necSpace(NEC_RPT_SPACE);
  necMark(NEC_BIT_MARK);
  interrupts();
}

void handleSerialCommand() {
  if (!Serial.available()) {
    return;
  }

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) {
    return;
  }

  char* endptr = nullptr;
  uint8_t cmd = static_cast<uint8_t>(strtoul(line.c_str(), &endptr, 0));
  if (endptr == line.c_str()) {
    Serial.println(F("Invalid command; enter hex or decimal (e.g., 0x14 or 20)"));
    return;
  }

  lastCommand = cmd;
  Serial.print(F("Sending NEC cmd 0x"));
  Serial.println(cmd, HEX);
  sendKenwood(cmd);
}

void necMark(uint16_t usec) {
  digitalWrite(NEC_PIN, HIGH);
  delayMicroseconds(usec);
}

void necSpace(uint16_t usec) {
  digitalWrite(NEC_PIN, LOW);
  delayMicroseconds(usec);
}

void necSendByte(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    necMark(NEC_BIT_MARK);
    if (data & 0x01) {
      necSpace(NEC_ONE_SPACE);
    } else {
      necSpace(NEC_ZERO_SPACE);
    }
    data >>= 1;
  }
}

void necSendFrame(uint8_t addr, uint8_t cmd) {
  uint8_t naddr = ~addr;
  uint8_t ncmd = ~cmd;

  noInterrupts();
  necMark(NEC_HDR_MARK);
  necSpace(NEC_HDR_SPACE);
  necSendByte(addr);
  necSendByte(naddr);
  necSendByte(cmd);
  necSendByte(ncmd);
  necMark(NEC_BIT_MARK);
  interrupts();
}
