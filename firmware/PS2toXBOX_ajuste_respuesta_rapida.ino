#include <PsxControllerBitBang.h>
#include <OGXBOX.h>

const uint8_t PIN_PS2_DAT = 2;
const uint8_t PIN_PS2_CMD = 3;
const uint8_t PIN_PS2_ATT = 4;
const uint8_t PIN_PS2_CLK = 5;

// BlueRetro in PS2 mode tends to feel better with a faster poll and without
// relying on pressure-button values for face buttons.
const uint8_t POLLING_HZ = 100U;
const unsigned long POLLING_INTERVAL_MS = 1000UL / POLLING_HZ;
const unsigned long RECONNECT_INTERVAL_MS = 250UL;
const unsigned long RUMBLE_RETRY_INTERVAL_MS = 1000UL;
const unsigned long RUMBLE_HOLD_MS = 24UL;

const uint8_t PSX_ANALOG_DEADZONE = 6U;
const uint8_t PSX_ANALOG_BUTTON_THRESHOLD = 8U;
const uint8_t MAX_CONSECUTIVE_READ_FAILURES = 3U;
const uint8_t RUMBLE_BIG_GAIN_PERCENT = 125U;

// Default tuning for BlueRetro -> PS2 -> Xbox:
// - face/D-pad buttons use instant digital states
// - triggers can stay analog if pressure values are valid
const bool FAST_DIGITAL_FACE_BUTTONS = true;
const bool FAST_DIGITAL_DPAD = true;
const bool FAST_DIGITAL_SHOULDERS = true;
const bool FAST_DIGITAL_TRIGGERS = false;

template <uint8_t ATT, uint8_t CMD, uint8_t DAT, uint8_t CLK>
class PsxControllerBitBangPatched : public PsxControllerBitBang<ATT, CMD, DAT, CLK> {
public:
  void forceRumbleTransport(const bool enabled) {
    this->rumbleEnabled = enabled;
  }

  bool analogSticksAreValid() const {
    return this->analogSticksValid;
  }

  bool analogButtonsAreValid() const {
    return this->analogButtonDataValid;
  }
};

PsxControllerBitBangPatched<PIN_PS2_ATT, PIN_PS2_CMD, PIN_PS2_DAT, PIN_PS2_CLK> psx;
OgXbox ogxbox(DUKE);

struct ControllerCapabilities {
  bool analogButtons;
  bool analogSticks;
  bool rumble;
};

static bool haveController = false;
static ControllerCapabilities caps = {false, false, false};
static uint8_t rumbleSmall = 0x00;
static uint8_t rumbleBig = 0x00;
static uint8_t consecutiveReadFailures = 0U;
static unsigned long lastConnectAttemptMs = 0UL;
static unsigned long lastRumblePacketMs = 0UL;
static unsigned long lastRumbleRetryMs = 0UL;

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

static inline bool analogPressed(const uint8_t value) {
  return value > PSX_ANALOG_BUTTON_THRESHOLD;
}

static inline uint8_t scaleBigRumble(const uint8_t value) {
  const uint16_t scaled = (static_cast<uint16_t>(value) * RUMBLE_BIG_GAIN_PERCENT) / 100U;
  return (scaled > 0xFFU) ? 0xFFU : static_cast<uint8_t>(scaled);
}

static inline bool protocolSupportsSticks(const PsxControllerProtocol protocol) {
  return protocol == PSPROTO_DUALSHOCK ||
         protocol == PSPROTO_DUALSHOCK2 ||
         protocol == PSPROTO_FLIGHTSTICK;
}

static inline void clearRumbleRequest() {
  rumbleSmall = 0x00;
  rumbleBig = 0x00;
  lastRumblePacketMs = 0UL;
}

static inline void applyStoppedRumble() {
  psx.setRumble(false, 0x00);
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
  caps = {false, false, false};
  consecutiveReadFailures = 0U;
  clearRumbleRequest();
  applyStoppedRumble();
  psx.forceRumbleTransport(false);
  resetXboxState();
}

bool primeControllerState() {
  if (!psx.read()) {
    return false;
  }

  const PsxControllerProtocol protocol = psx.getProtocol();
  caps.analogSticks = caps.analogSticks && psx.analogSticksAreValid() && protocolSupportsSticks(protocol);
  caps.analogButtons = caps.analogButtons && psx.analogButtonsAreValid() && (protocol == PSPROTO_DUALSHOCK2);
  return true;
}

bool connectController() {
  caps = {false, false, false};

  if (!psx.begin()) {
    psx.forceRumbleTransport(false);
    return false;
  }

  if (!psx.enterConfigMode()) {
    psx.forceRumbleTransport(false);
    return false;
  }

  caps.analogSticks = psx.enableAnalogSticks();
  caps.analogButtons = psx.enableAnalogButtons();
  caps.rumble = psx.enableRumble();
  psx.forceRumbleTransport(caps.rumble);

  if (!psx.exitConfigMode()) {
    caps = {false, false, false};
    psx.forceRumbleTransport(false);
    return false;
  }

  if (!primeControllerState()) {
    caps = {false, false, false};
    psx.forceRumbleTransport(false);
    return false;
  }

  haveController = true;
  consecutiveReadFailures = 0U;
  clearRumbleRequest();
  applyStoppedRumble();
  resetXboxState();
  return true;
}

bool ensureRumbleReady(const unsigned long now) {
  if (caps.rumble) {
    return true;
  }

  if ((now - lastRumbleRetryMs) < RUMBLE_RETRY_INTERVAL_MS) {
    return false;
  }

  lastRumbleRetryMs = now;

  if (!psx.enterConfigMode()) {
    psx.forceRumbleTransport(false);
    return false;
  }

  const bool rumbleOk = psx.enableRumble();
  psx.forceRumbleTransport(rumbleOk);

  if (!psx.exitConfigMode()) {
    psx.forceRumbleTransport(false);
    return false;
  }

  if (!rumbleOk) {
    psx.forceRumbleTransport(false);
    return false;
  }

  if (!primeControllerState()) {
    psx.forceRumbleTransport(false);
    return false;
  }

  caps.rumble = true;
  return true;
}

void applyRumbleRequest(const unsigned long now) {
  if ((rumbleSmall | rumbleBig) == 0x00) {
    applyStoppedRumble();
    return;
  }

  if (!caps.rumble && !ensureRumbleReady(now)) {
    applyStoppedRumble();
    return;
  }

  psx.setRumble(rumbleSmall > 0x00, rumbleBig);
}

void refreshRumbleStateFromXbox(const unsigned long now) {
  uint8_t newRumbleSmall = 0x00;
  uint8_t newRumbleBig = 0x00;

  if (ogxbox.getRumble(newRumbleSmall, newRumbleBig)) {
    rumbleSmall = newRumbleSmall;
    rumbleBig = scaleBigRumble(newRumbleBig);
    lastRumblePacketMs = now;
    return;
  }

  if ((rumbleSmall | rumbleBig) != 0x00 &&
      lastRumblePacketMs != 0UL &&
      (now - lastRumblePacketMs) > RUMBLE_HOLD_MS) {
    clearRumbleRequest();
  }
}

void updateButtonsFastDigital() {
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

  if (FAST_DIGITAL_TRIGGERS) {
    ogxbox.setLeftTrigger(psx.buttonPressed(PSB_L2) ? 0xFF : 0x00);
    ogxbox.setRightTrigger(psx.buttonPressed(PSB_R2) ? 0xFF : 0x00);
  } else {
    ogxbox.setLeftTrigger(caps.analogButtons ? psx.getAnalogButton(PSAB_L2) : (psx.buttonPressed(PSB_L2) ? 0xFF : 0x00));
    ogxbox.setRightTrigger(caps.analogButtons ? psx.getAnalogButton(PSAB_R2) : (psx.buttonPressed(PSB_R2) ? 0xFF : 0x00));
  }
}

void updateButtonsHybridAnalog() {
  if (FAST_DIGITAL_DPAD) {
    ogxbox.setDpad(psx.buttonPressed(PSB_PAD_UP),
                   psx.buttonPressed(PSB_PAD_DOWN),
                   psx.buttonPressed(PSB_PAD_LEFT),
                   psx.buttonPressed(PSB_PAD_RIGHT));
  } else {
    const uint8_t up = psx.getAnalogButton(PSAB_PAD_UP);
    const uint8_t down = psx.getAnalogButton(PSAB_PAD_DOWN);
    const uint8_t left = psx.getAnalogButton(PSAB_PAD_LEFT);
    const uint8_t right = psx.getAnalogButton(PSAB_PAD_RIGHT);

    ogxbox.setDpad(analogPressed(up),
                   analogPressed(down),
                   analogPressed(left),
                   analogPressed(right));
  }

  if (FAST_DIGITAL_FACE_BUTTONS) {
    ogxbox.pressAnalogButton(ANA_BTN_A, psx.buttonPressed(PSB_CROSS) ? 0xFF : 0x00);
    ogxbox.pressAnalogButton(ANA_BTN_B, psx.buttonPressed(PSB_CIRCLE) ? 0xFF : 0x00);
    ogxbox.pressAnalogButton(ANA_BTN_X, psx.buttonPressed(PSB_SQUARE) ? 0xFF : 0x00);
    ogxbox.pressAnalogButton(ANA_BTN_Y, psx.buttonPressed(PSB_TRIANGLE) ? 0xFF : 0x00);
  } else {
    ogxbox.pressAnalogButton(ANA_BTN_A, psx.getAnalogButton(PSAB_CROSS));
    ogxbox.pressAnalogButton(ANA_BTN_B, psx.getAnalogButton(PSAB_CIRCLE));
    ogxbox.pressAnalogButton(ANA_BTN_X, psx.getAnalogButton(PSAB_SQUARE));
    ogxbox.pressAnalogButton(ANA_BTN_Y, psx.getAnalogButton(PSAB_TRIANGLE));
  }

  if (FAST_DIGITAL_SHOULDERS) {
    ogxbox.pressAnalogButton(ANA_BTN_BLACK, psx.buttonPressed(PSB_R1) ? 0xFF : 0x00);
    ogxbox.pressAnalogButton(ANA_BTN_WHITE, psx.buttonPressed(PSB_L1) ? 0xFF : 0x00);
  } else {
    ogxbox.pressAnalogButton(ANA_BTN_BLACK, psx.getAnalogButton(PSAB_R1));
    ogxbox.pressAnalogButton(ANA_BTN_WHITE, psx.getAnalogButton(PSAB_L1));
  }

  if (FAST_DIGITAL_TRIGGERS) {
    ogxbox.setLeftTrigger(psx.buttonPressed(PSB_L2) ? 0xFF : 0x00);
    ogxbox.setRightTrigger(psx.buttonPressed(PSB_R2) ? 0xFF : 0x00);
  } else {
    ogxbox.setLeftTrigger(psx.getAnalogButton(PSAB_L2));
    ogxbox.setRightTrigger(psx.getAnalogButton(PSAB_R2));
  }
}

void updateMetaButtons() {
  ogxbox.setButton(BUTTON_START, psx.buttonPressed(PSB_START));
  ogxbox.setButton(BUTTON_BACK, psx.buttonPressed(PSB_SELECT));
  ogxbox.setButton(BUTTON_LS, psx.buttonPressed(PSB_L3));
  ogxbox.setButton(BUTTON_RS, psx.buttonPressed(PSB_R3));
}

void updateSticks() {
  const PsxControllerProtocol protocol = psx.getProtocol();
  const bool sticksValid = caps.analogSticks && psx.analogSticksAreValid() && protocolSupportsSticks(protocol);

  if (!sticksValid) {
    ogxbox.setLeftJoystick(0, 0);
    ogxbox.setRightJoystick(0, 0);
    return;
  }

  uint8_t axisX = ANALOG_IDLE_VALUE;
  uint8_t axisY = ANALOG_IDLE_VALUE;

  psx.getLeftAnalog(axisX, axisY);
  ogxbox.setLeftJoystick(mapPsxAxisToXbox(axisX), mapPsxAxisToXboxInverted(axisY));

  psx.getRightAnalog(axisX, axisY);
  ogxbox.setRightJoystick(mapPsxAxisToXbox(axisX), mapPsxAxisToXboxInverted(axisY));
}

void setup() {
  ogxbox.begin();
  resetXboxState();
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

  applyRumbleRequest(now);

  if (!psx.read()) {
    applyStoppedRumble();

    if (++consecutiveReadFailures >= MAX_CONSECUTIVE_READ_FAILURES) {
      disconnectController();
    }
    return;
  }

  consecutiveReadFailures = 0U;

  const bool useAnalogButtons = caps.analogButtons &&
                                psx.analogButtonsAreValid() &&
                                (psx.getProtocol() == PSPROTO_DUALSHOCK2);

  if (useAnalogButtons) {
    updateButtonsHybridAnalog();
  } else {
    updateButtonsFastDigital();
  }

  updateMetaButtons();
  updateSticks();

  ogxbox.sendReport();
  refreshRumbleStateFromXbox(now);
}
