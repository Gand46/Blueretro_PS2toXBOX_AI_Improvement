#include <PsxControllerBitBang.h>
#include <OGXBOX.h>

constexpr uint8_t PIN_PS2_DAT = 2;
constexpr uint8_t PIN_PS2_CMD = 3;
constexpr uint8_t PIN_PS2_ATT = 4;
constexpr uint8_t PIN_PS2_CLK = 5;

// 250 Hz scan/update loop keeps end-to-end latency low while remaining stable on ATmega32u4.
constexpr uint32_t POLLING_INTERVAL_US = 4000UL;

PsxControllerBitBang<PIN_PS2_ATT, PIN_PS2_CMD, PIN_PS2_DAT, PIN_PS2_CLK> psx;
OgXbox ogxbox(DUKE);

bool haveController = false;
bool enableAnalogButton = false;
bool enableAnalogSticks = false;
bool enableRumble = false;
bool okRumble = false;
uint8_t lValue = 0;
uint8_t hValue = 0;

static inline int16_t mapAxisToDuke(const uint8_t value, const bool invert) {
  // Equivalent to Arduino map(0..255 -> -32768..32767), but faster and without long math.
  int16_t mapped = static_cast<int16_t>((static_cast<uint16_t>(value) << 8) - 32768);
  return invert ? static_cast<int16_t>(-mapped - 1) : mapped;
}

static inline void sendNeutralReport() {
  ogxbox.setDpad(false, false, false, false);
  ogxbox.setButton(BUTTON_A, false);
  ogxbox.setButton(BUTTON_B, false);
  ogxbox.setButton(BUTTON_X, false);
  ogxbox.setButton(BUTTON_Y, false);
  ogxbox.setButton(BUTTON_BLACK, false);
  ogxbox.setButton(BUTTON_WHITE, false);
  ogxbox.setButton(BUTTON_START, false);
  ogxbox.setButton(BUTTON_BACK, false);
  ogxbox.setButton(BUTTON_LS, false);
  ogxbox.setButton(BUTTON_RS, false);
  ogxbox.setLeftTrigger(0x00);
  ogxbox.setRightTrigger(0x00);
  ogxbox.setLeftJoystick(0, 0);
  ogxbox.setRightJoystick(0, 0);
  ogxbox.sendReport();
}

static inline void configureController() {
  psx.enterConfigMode();
  enableAnalogSticks = psx.enableAnalogSticks();
  enableAnalogButton = psx.enableAnalogButtons();
  enableRumble = psx.enableRumble();
  psx.exitConfigMode();
}

void setup() {
  ogxbox.begin();
}

void loop() {
  static uint32_t nextPollUs = 0;

  const uint32_t nowUs = micros();

  if (nextPollUs == 0) {
    nextPollUs = nowUs + POLLING_INTERVAL_US;
    return;
  }

  if (static_cast<int32_t>(nowUs - nextPollUs) < 0) {
    return;
  }

  // Keep cadence deterministic and avoid jitter/drift under occasional overruns.
  do {
    nextPollUs += POLLING_INTERVAL_US;
  } while (static_cast<int32_t>(nowUs - nextPollUs) >= 0);

  if (!haveController) {
    if (psx.begin()) {
      configureController();
      haveController = true;
    }
    else {
      sendNeutralReport();
    }
    return;
  }

  if (!psx.read()) {
    haveController = false;
    okRumble = false;
    sendNeutralReport();
    return;
  }

  psx.setRumble(enableRumble && okRumble && (lValue > 0x00), (enableRumble && okRumble) ? hValue : 0x00);

  if (enableAnalogButton) {
    const uint8_t padUp = psx.getAnalogButton(PSAB_PAD_UP);
    const uint8_t padDown = psx.getAnalogButton(PSAB_PAD_DOWN);
    const uint8_t padLeft = psx.getAnalogButton(PSAB_PAD_LEFT);
    const uint8_t padRight = psx.getAnalogButton(PSAB_PAD_RIGHT);

    ogxbox.setDpad(padUp > 0x00, padDown > 0x00, padLeft > 0x00, padRight > 0x00);

    ogxbox.pressAnalogButton(ANA_BTN_A, psx.getAnalogButton(PSAB_CROSS));
    ogxbox.pressAnalogButton(ANA_BTN_B, psx.getAnalogButton(PSAB_CIRCLE));
    ogxbox.pressAnalogButton(ANA_BTN_X, psx.getAnalogButton(PSAB_SQUARE));
    ogxbox.pressAnalogButton(ANA_BTN_Y, psx.getAnalogButton(PSAB_TRIANGLE));
    ogxbox.pressAnalogButton(ANA_BTN_BLACK, psx.getAnalogButton(PSAB_R1));
    ogxbox.pressAnalogButton(ANA_BTN_WHITE, psx.getAnalogButton(PSAB_L1));

    ogxbox.setLeftTrigger(psx.getAnalogButton(PSAB_L2));
    ogxbox.setRightTrigger(psx.getAnalogButton(PSAB_R2));
  }
  else {
    const bool padUp = psx.buttonPressed(PSB_PAD_UP);
    const bool padDown = psx.buttonPressed(PSB_PAD_DOWN);
    const bool padLeft = psx.buttonPressed(PSB_PAD_LEFT);
    const bool padRight = psx.buttonPressed(PSB_PAD_RIGHT);

    ogxbox.setDpad(padUp, padDown, padLeft, padRight);

    ogxbox.setButton(BUTTON_A, psx.buttonPressed(PSB_CROSS));
    ogxbox.setButton(BUTTON_B, psx.buttonPressed(PSB_CIRCLE));
    ogxbox.setButton(BUTTON_X, psx.buttonPressed(PSB_SQUARE));
    ogxbox.setButton(BUTTON_Y, psx.buttonPressed(PSB_TRIANGLE));
    ogxbox.setButton(BUTTON_BLACK, psx.buttonPressed(PSB_R1));
    ogxbox.setButton(BUTTON_WHITE, psx.buttonPressed(PSB_L1));

    ogxbox.setLeftTrigger(psx.buttonPressed(PSB_L2) ? 0xFF : 0x00);
    ogxbox.setRightTrigger(psx.buttonPressed(PSB_R2) ? 0xFF : 0x00);
  }

  ogxbox.setButton(BUTTON_START, psx.buttonPressed(PSB_START));
  ogxbox.setButton(BUTTON_BACK, psx.buttonPressed(PSB_SELECT));
  ogxbox.setButton(BUTTON_LS, psx.buttonPressed(PSB_L3));
  ogxbox.setButton(BUTTON_RS, psx.buttonPressed(PSB_R3));

  if (enableAnalogSticks) {
    uint8_t axisX = 0;
    uint8_t axisY = 0;

    psx.getLeftAnalog(axisX, axisY);
    ogxbox.setLeftJoystick(mapAxisToDuke(axisX, false), mapAxisToDuke(axisY, true));

    psx.getRightAnalog(axisX, axisY);
    ogxbox.setRightJoystick(mapAxisToDuke(axisX, false), mapAxisToDuke(axisY, true));
  }
  else {
    ogxbox.setLeftJoystick(0, 0);
    ogxbox.setRightJoystick(0, 0);
  }

  ogxbox.sendReport();
  okRumble = ogxbox.getRumble(lValue, hValue);
}
