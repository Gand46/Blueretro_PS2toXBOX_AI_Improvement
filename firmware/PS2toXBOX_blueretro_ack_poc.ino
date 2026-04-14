#include <PsxNewLib.h>
#include <OGXBOX.h>

// ACK PoC profile (requires PsxNewLib devel branch + HW SPI wiring).
// SPI pins are board-defined (ATmega32u4 ICSP lines). Only ATT/ACK are configurable.
const uint8_t PIN_PS2_ATT = 4;
const uint8_t PIN_PS2_ACK = 6;

const uint8_t POLLING_HZ = 100U;
const unsigned long POLLING_INTERVAL_MS = 1000UL / POLLING_HZ;
const unsigned long RECONNECT_INTERVAL_MS = 250UL;
const uint8_t MAX_CONSECUTIVE_READ_FAILURES = 3U;
const uint8_t PSX_ANALOG_DEADZONE = 6U;

// BlueRetro pressure-button emulation can be noisy in some setups.
const bool USE_ANALOG_FACE_BUTTONS = false;
const bool USE_ANALOG_DPAD = false;
const bool USE_ANALOG_SHOULDERS = false;
const bool USE_ANALOG_TRIGGERS = false;

PsxDriverHwSpiWithAck<PIN_PS2_ATT, PIN_PS2_ACK> psxDriver;
PsxSingleController psx;
OgXbox ogxbox(DUKE);

struct ControllerCapabilities {
  bool analogButtons;
  bool analogSticks;
};

static bool haveController = false;
static ControllerCapabilities caps = {false, false};
static uint8_t consecutiveReadFailures = 0U;
static unsigned long lastConnectAttemptMs = 0UL;

static inline int16_t clampAxis(const long value) {
  if (value < DUKE_JOYSTICK_MIN) {
    return DUKE_JOYSTICK_MIN;
  }
  if (value > DUKE_JOYSTICK_MAX) {
    return DUKE_JOYSTICK_MAX;
  }
  return static_cast<int16_t>(value);
}

static inline int16_t mapPsxAxisToXbox(const uint8_t value) {
  const int16_t centered = static_cast<int16_t>(value) - static_cast<int16_t>(ANALOG_IDLE_VALUE);

  if (centered >= -static_cast<int16_t>(PSX_ANALOG_DEADZONE) &&
      centered <= static_cast<int16_t>(PSX_ANALOG_DEADZONE)) {
    return 0;
  }

  if (centered >= 0) {
    return clampAxis((static_cast<long>(centered) * static_cast<long>(DUKE_JOYSTICK_MAX)) /
                     static_cast<long>(ANALOG_MAX_VALUE - ANALOG_IDLE_VALUE));
  }

  return clampAxis((static_cast<long>(centered) * static_cast<long>(-DUKE_JOYSTICK_MIN)) /
                   static_cast<long>(ANALOG_IDLE_VALUE));
}

static inline int16_t mapPsxAxisToXboxInverted(const uint8_t value) {
  return clampAxis(-static_cast<long>(mapPsxAxisToXbox(value)));
}

static inline bool protocolSupportsSticks(const PsxControllerProtocol protocol) {
  return protocol == PSPROTO_DUALSHOCK ||
         protocol == PSPROTO_DUALSHOCK2 ||
         protocol == PSPROTO_FLIGHTSTICK;
}

static inline bool protocolSupportsAnalogButtons(const PsxControllerProtocol protocol) {
  return protocol == PSPROTO_DUALSHOCK2;
}

void resetXboxState() {
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

void disconnectController() {
  haveController = false;
  caps = {false, false};
  consecutiveReadFailures = 0U;
  resetXboxState();
}

bool primeControllerState() {
  if (!psx.read()) {
    return false;
  }

  const PsxControllerProtocol protocol = psx.getProtocol();
  caps.analogSticks = protocolSupportsSticks(protocol);
  caps.analogButtons = protocolSupportsAnalogButtons(protocol) && (psx.getAnalogButtonData() != nullptr);
  return true;
}

bool connectController() {
  caps = {false, false};

  if (!psx.begin(psxDriver)) {
    return false;
  }

  if (!psx.enterConfigMode()) {
    return false;
  }

  caps.analogSticks = psx.enableAnalogSticks();
  caps.analogButtons = psx.enableAnalogButtons();

  if (!psx.exitConfigMode()) {
    caps = {false, false};
    return false;
  }

  if (!primeControllerState()) {
    caps = {false, false};
    return false;
  }

  haveController = true;
  consecutiveReadFailures = 0U;
  resetXboxState();
  return true;
}

void updateDigitalButtons() {
  ogxbox.setDpad(psx.buttonPressed(PSB_PAD_UP),
                 psx.buttonPressed(PSB_PAD_DOWN),
                 psx.buttonPressed(PSB_PAD_LEFT),
                 psx.buttonPressed(PSB_PAD_RIGHT));

  ogxbox.pressAnalogButton(ANA_BTN_A, psx.buttonPressed(PSB_CROSS) ? 0xFF : 0x00);
  ogxbox.pressAnalogButton(ANA_BTN_B, psx.buttonPressed(PSB_CIRCLE) ? 0xFF : 0x00);
  ogxbox.pressAnalogButton(ANA_BTN_X, psx.buttonPressed(PSB_SQUARE) ? 0xFF : 0x00);
  ogxbox.pressAnalogButton(ANA_BTN_Y, psx.buttonPressed(PSB_TRIANGLE) ? 0xFF : 0x00);
  ogxbox.pressAnalogButton(ANA_BTN_BLACK, psx.buttonPressed(PSB_R1) ? 0xFF : 0x00);
  ogxbox.pressAnalogButton(ANA_BTN_WHITE, psx.buttonPressed(PSB_L1) ? 0xFF : 0x00);
  ogxbox.setLeftTrigger(psx.buttonPressed(PSB_L2) ? 0xFF : 0x00);
  ogxbox.setRightTrigger(psx.buttonPressed(PSB_R2) ? 0xFF : 0x00);
}

void updateAnalogButtonsIfEnabled() {
  if (!caps.analogButtons || !protocolSupportsAnalogButtons(psx.getProtocol()) || psx.getAnalogButtonData() == nullptr) {
    updateDigitalButtons();
    return;
  }

  if (USE_ANALOG_DPAD) {
    ogxbox.setDpad(psx.getAnalogButton(PSAB_PAD_UP) > 0x00,
                   psx.getAnalogButton(PSAB_PAD_DOWN) > 0x00,
                   psx.getAnalogButton(PSAB_PAD_LEFT) > 0x00,
                   psx.getAnalogButton(PSAB_PAD_RIGHT) > 0x00);
  } else {
    ogxbox.setDpad(psx.buttonPressed(PSB_PAD_UP),
                   psx.buttonPressed(PSB_PAD_DOWN),
                   psx.buttonPressed(PSB_PAD_LEFT),
                   psx.buttonPressed(PSB_PAD_RIGHT));
  }

  if (USE_ANALOG_FACE_BUTTONS) {
    ogxbox.pressAnalogButton(ANA_BTN_A, psx.getAnalogButton(PSAB_CROSS));
    ogxbox.pressAnalogButton(ANA_BTN_B, psx.getAnalogButton(PSAB_CIRCLE));
    ogxbox.pressAnalogButton(ANA_BTN_X, psx.getAnalogButton(PSAB_SQUARE));
    ogxbox.pressAnalogButton(ANA_BTN_Y, psx.getAnalogButton(PSAB_TRIANGLE));
  } else {
    ogxbox.pressAnalogButton(ANA_BTN_A, psx.buttonPressed(PSB_CROSS) ? 0xFF : 0x00);
    ogxbox.pressAnalogButton(ANA_BTN_B, psx.buttonPressed(PSB_CIRCLE) ? 0xFF : 0x00);
    ogxbox.pressAnalogButton(ANA_BTN_X, psx.buttonPressed(PSB_SQUARE) ? 0xFF : 0x00);
    ogxbox.pressAnalogButton(ANA_BTN_Y, psx.buttonPressed(PSB_TRIANGLE) ? 0xFF : 0x00);
  }

  if (USE_ANALOG_SHOULDERS) {
    ogxbox.pressAnalogButton(ANA_BTN_BLACK, psx.getAnalogButton(PSAB_R1));
    ogxbox.pressAnalogButton(ANA_BTN_WHITE, psx.getAnalogButton(PSAB_L1));
  } else {
    ogxbox.pressAnalogButton(ANA_BTN_BLACK, psx.buttonPressed(PSB_R1) ? 0xFF : 0x00);
    ogxbox.pressAnalogButton(ANA_BTN_WHITE, psx.buttonPressed(PSB_L1) ? 0xFF : 0x00);
  }

  if (USE_ANALOG_TRIGGERS) {
    ogxbox.setLeftTrigger(psx.getAnalogButton(PSAB_L2));
    ogxbox.setRightTrigger(psx.getAnalogButton(PSAB_R2));
  } else {
    ogxbox.setLeftTrigger(psx.buttonPressed(PSB_L2) ? 0xFF : 0x00);
    ogxbox.setRightTrigger(psx.buttonPressed(PSB_R2) ? 0xFF : 0x00);
  }
}

void updateMetaButtons() {
  ogxbox.setButton(BUTTON_START, psx.buttonPressed(PSB_START));
  ogxbox.setButton(BUTTON_BACK, psx.buttonPressed(PSB_SELECT));
  ogxbox.setButton(BUTTON_LS, psx.buttonPressed(PSB_L3));
  ogxbox.setButton(BUTTON_RS, psx.buttonPressed(PSB_R3));
}

void updateSticks() {
  uint8_t axisX = ANALOG_IDLE_VALUE;
  uint8_t axisY = ANALOG_IDLE_VALUE;

  if (!caps.analogSticks || !protocolSupportsSticks(psx.getProtocol()) || !psx.getLeftAnalog(axisX, axisY)) {
    ogxbox.setLeftJoystick(0, 0);
  } else {
    ogxbox.setLeftJoystick(mapPsxAxisToXbox(axisX), mapPsxAxisToXboxInverted(axisY));
  }

  if (!caps.analogSticks || !protocolSupportsSticks(psx.getProtocol()) || !psx.getRightAnalog(axisX, axisY)) {
    ogxbox.setRightJoystick(0, 0);
  } else {
    ogxbox.setRightJoystick(mapPsxAxisToXbox(axisX), mapPsxAxisToXboxInverted(axisY));
  }
}

void setup() {
  ogxbox.begin();
  resetXboxState();
  psxDriver.begin();
}

void loop() {
  static unsigned long lastPollMs = 0UL;
  const unsigned long now = millis();

  if ((now - lastPollMs) < POLLING_INTERVAL_MS) {
    return;
  }

  lastPollMs = now;

  if (!haveController) {
    if ((now - lastConnectAttemptMs) >= RECONNECT_INTERVAL_MS) {
      lastConnectAttemptMs = now;
      connectController();
    }
    return;
  }

  if (!psx.read()) {
    if (++consecutiveReadFailures >= MAX_CONSECUTIVE_READ_FAILURES) {
      disconnectController();
    }
    return;
  }

  consecutiveReadFailures = 0U;

  updateAnalogButtonsIfEnabled();
  updateMetaButtons();
  updateSticks();

  // ACK PoC on devel branch currently focuses on input reliability.
  // Rumble is intentionally not managed in this variant.
  ogxbox.sendReport();
}
