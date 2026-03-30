#include <PsxControllerBitBang.h>
#include <OGXBOX.h>

constexpr uint8_t PIN_PS2_DAT = 2;
constexpr uint8_t PIN_PS2_CMD = 3;
constexpr uint8_t PIN_PS2_ATT = 4;
constexpr uint8_t PIN_PS2_CLK = 5;

constexpr uint32_t POLLING_INTERVAL_US = 4000UL;
constexpr uint8_t MAX_CONSECUTIVE_BEGIN_FAILS = 25;
constexpr uint8_t MAX_CONSECUTIVE_READ_FAILS = 5;

constexpr bool ENABLE_TIMING_INSTRUMENTATION = true;
constexpr uint8_t PIN_PROBE_LOOP = 7;
constexpr uint8_t PIN_PROBE_PSX_READ = 8;
constexpr uint8_t PIN_PROBE_USB_SEND = 9;

PsxControllerBitBang<PIN_PS2_ATT, PIN_PS2_CMD, PIN_PS2_DAT, PIN_PS2_CLK> psx;
OgXbox ogxbox(DUKE);

enum class ControllerState : uint8_t {
  DISCOVERY,
  CONFIGURING,
  ACTIVE,
  RECOVERING
};

enum class FaultCode : uint8_t {
  NONE,
  BEGIN_FAIL,
  CONFIG_READ_FAIL,
  READ_FAIL,
  DEADLINE_OVERRUN
};

struct RuntimeStats {
  uint16_t missedDeadlines;
  uint16_t consecutiveBeginFails;
  uint16_t consecutiveReadFails;
  uint32_t maxLoopUs;
  uint32_t maxReadUs;
  uint32_t maxReportUs;
  FaultCode lastFault;
};

ControllerState state = ControllerState::DISCOVERY;
RuntimeStats stats = {0, 0, 0, 0, 0, 0, FaultCode::NONE};

bool enableAnalogButton = false;
bool enableAnalogSticks = false;
bool enableRumble = false;
bool okRumble = false;
uint8_t lValue = 0;
uint8_t hValue = 0;

static inline void probeBegin(const uint8_t pin) {
  if (ENABLE_TIMING_INSTRUMENTATION) {
    digitalWrite(pin, HIGH);
  }
}

static inline void probeEnd(const uint8_t pin) {
  if (ENABLE_TIMING_INSTRUMENTATION) {
    digitalWrite(pin, LOW);
  }
}

static inline void setFault(const FaultCode fault) {
  stats.lastFault = fault;
}

static inline int16_t mapAxisToDuke(const uint8_t value, const bool invert) {
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

  const uint32_t reportStart = micros();
  probeBegin(PIN_PROBE_USB_SEND);
  ogxbox.sendReport();
  probeEnd(PIN_PROBE_USB_SEND);
  const uint32_t reportElapsed = micros() - reportStart;

  if (reportElapsed > stats.maxReportUs) {
    stats.maxReportUs = reportElapsed;
  }
}

static inline bool configureController() {
  psx.enterConfigMode();
  enableAnalogSticks = psx.enableAnalogSticks();
  enableAnalogButton = psx.enableAnalogButtons();
  enableRumble = psx.enableRumble();
  psx.exitConfigMode();

  probeBegin(PIN_PROBE_PSX_READ);
  const bool configuredAndReadable = psx.read();
  probeEnd(PIN_PROBE_PSX_READ);
  return configuredAndReadable;
}

static inline void mapAndSendReport() {
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
    ogxbox.setDpad(psx.buttonPressed(PSB_PAD_UP),
                   psx.buttonPressed(PSB_PAD_DOWN),
                   psx.buttonPressed(PSB_PAD_LEFT),
                   psx.buttonPressed(PSB_PAD_RIGHT));

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

  const uint32_t reportStart = micros();
  probeBegin(PIN_PROBE_USB_SEND);
  ogxbox.sendReport();
  probeEnd(PIN_PROBE_USB_SEND);
  const uint32_t reportElapsed = micros() - reportStart;
  if (reportElapsed > stats.maxReportUs) {
    stats.maxReportUs = reportElapsed;
  }

  okRumble = ogxbox.getRumble(lValue, hValue);
}

void setup() {
  ogxbox.begin();

  if (ENABLE_TIMING_INSTRUMENTATION) {
    pinMode(PIN_PROBE_LOOP, OUTPUT);
    pinMode(PIN_PROBE_PSX_READ, OUTPUT);
    pinMode(PIN_PROBE_USB_SEND, OUTPUT);
    digitalWrite(PIN_PROBE_LOOP, LOW);
    digitalWrite(PIN_PROBE_PSX_READ, LOW);
    digitalWrite(PIN_PROBE_USB_SEND, LOW);
  }
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

  if ((nowUs - nextPollUs) > POLLING_INTERVAL_US) {
    stats.missedDeadlines++;
    setFault(FaultCode::DEADLINE_OVERRUN);
  }

  do {
    nextPollUs += POLLING_INTERVAL_US;
  } while (static_cast<int32_t>(nowUs - nextPollUs) >= 0);

  const uint32_t cycleStart = micros();
  probeBegin(PIN_PROBE_LOOP);

  switch (state) {
    case ControllerState::DISCOVERY:
      if (psx.begin()) {
        stats.consecutiveBeginFails = 0;
        state = ControllerState::CONFIGURING;
      }
      else {
        stats.consecutiveBeginFails++;
        setFault(FaultCode::BEGIN_FAIL);
        sendNeutralReport();
        if (stats.consecutiveBeginFails >= MAX_CONSECUTIVE_BEGIN_FAILS) {
          stats.consecutiveBeginFails = 0;
        }
      }
      break;

    case ControllerState::CONFIGURING:
      if (!configureController()) {
        setFault(FaultCode::CONFIG_READ_FAIL);
        state = ControllerState::RECOVERING;
        sendNeutralReport();
        break;
      }
      stats.consecutiveReadFails = 0;
      state = ControllerState::ACTIVE;
      break;

    case ControllerState::ACTIVE: {
      probeBegin(PIN_PROBE_PSX_READ);
      const uint32_t readStart = micros();
      const bool readOk = psx.read();
      const uint32_t readElapsed = micros() - readStart;
      probeEnd(PIN_PROBE_PSX_READ);

      if (readElapsed > stats.maxReadUs) {
        stats.maxReadUs = readElapsed;
      }

      if (!readOk) {
        stats.consecutiveReadFails++;
        setFault(FaultCode::READ_FAIL);
        if (stats.consecutiveReadFails >= MAX_CONSECUTIVE_READ_FAILS) {
          state = ControllerState::RECOVERING;
          sendNeutralReport();
        }
        break;
      }

      stats.consecutiveReadFails = 0;
      psx.setRumble(enableRumble && okRumble && (lValue > 0x00),
                    (enableRumble && okRumble) ? hValue : 0x00);
      mapAndSendReport();
      break;
    }

    case ControllerState::RECOVERING:
      enableAnalogButton = false;
      enableAnalogSticks = false;
      enableRumble = false;
      okRumble = false;
      lValue = 0;
      hValue = 0;
      sendNeutralReport();
      state = ControllerState::DISCOVERY;
      break;
  }

  probeEnd(PIN_PROBE_LOOP);
  const uint32_t cycleElapsed = micros() - cycleStart;
  if (cycleElapsed > stats.maxLoopUs) {
    stats.maxLoopUs = cycleElapsed;
  }
}
