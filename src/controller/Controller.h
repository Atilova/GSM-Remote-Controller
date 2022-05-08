#ifndef CONTROLLER_H_
  #define CONTROLLER_H_

  #include <TinyGsmClientSIM800.h>
  #include <ThreadHandler.h>

  

  // 1000 -> 1ms
  #define BLINK_PERIOD 300000
  #define LOOP_PERIOD 100000

  typedef TinyGsmSim800 TinyGsm;

  enum makeshiftTimerState {
    ACTIVE,
    DISABLED
  };

  struct controllerConfig {
    const char** phoneWhitelist;
    const uint8_t phoneWhitelistQuantity;
    const uint8_t resetPin;
    const uint8_t relayPin;
    const uint8_t swapModePin;
    boolean isMakeshitPower;
    const boolean isOnByDefaultMakeshift;
    const uint8_t makeShiftPowerDuration;  // In seconds
    const boolean isLowLevelRelay;
  };

  class SystemController {
    static SystemController* _instance;
    static void swapModeISR(void);
    static void blinkLed(void);
    static void loop(void);

    public:
      SystemController(TinyGsm& modem, controllerConfig& config);
      ~SystemController();      
      void begin(void);

    private:
      TinyGsm* _modem;
      controllerConfig* _config;
      boolean relayState = true;  // If Low level (default) will be down (relay)
      uint32_t makeshiftPowerTimer,
               lastTimePressed;
      makeshiftTimerState timerState = DISABLED;
      void resetModem(void);
      void initModem(void);
      void setNextRelayState(void);
      void resetMakeshiftTimer(void);      
      void setMakeShiftPower(boolean isMakeshift);
      void setRelayState(boolean relayState);
      boolean isWhiteListed(const String& phone);
      Thread* systemThread = createThread(0, LOOP_PERIOD, 0, SystemController::loop);
      Thread* ledThread = createThread(1, BLINK_PERIOD, 0, SystemController::blinkLed);
  };    
#endif