#include "Arduino.h"
#include "Controller.h"


SystemController::SystemController(TinyGsm& modem, controllerConfig& _config) {    
  this -> _modem = &modem;   
  this -> _config = &_config;
}

void SystemController::swapModeISR(void) {
  if(millis() > _instance -> lastTimePressed) {
    _instance -> lastTimePressed = millis() + 200;
    _instance -> setMakeShiftPower(!_instance -> _config -> isMakeshitPower);
  }      
}

void SystemController::begin(void) {  
  pinMode(_config -> relayPin, OUTPUT);
  pinMode(_config -> swapModePin, INPUT_PULLUP);
  if(!_config -> isLowLevelRelay)
    relayState = false;  
      
  attachInterrupt(
    digitalPinToInterrupt(_config -> swapModePin), 
    SystemController::swapModeISR,
    RISING
  );
  _instance = this;

  digitalWrite(_config -> relayPin, relayState);  // DOWN by default
  resetModem();
  while(!_modem -> waitForNetwork()) {
    delay(1000);
  }

  ThreadHandler::getInstance()->enableThreadExecution();
}

void SystemController::setRelayState(boolean relayState) {
  const boolean nextState = (
    relayState
      ? !_config -> isLowLevelRelay 
      : _config -> isLowLevelRelay
  );
  
  relayState = nextState;
  digitalWrite(
    _config -> relayPin,
    nextState
  );
};

void SystemController::resetModem(void) {
  digitalWrite(_config -> resetPin, HIGH);
  pinMode(_config -> resetPin, OUTPUT);
  delay(100);
  digitalWrite(_config -> resetPin, LOW);
  delay(400);
  digitalWrite(_config -> resetPin, HIGH);
  initModem();
}

void SystemController::initModem(void) {
  _modem -> init();
  _modem -> sendAT(GF(""));   // Check Status (AT)
  _modem -> waitResponse(1000L, GF("OK"));
  _modem -> sendAT(GF("E0"));  // Echo OFF, ON(ATE1)
  _modem -> waitResponse(1000L, GF("OK"));
  _modem -> sendAT(GF("+COLP=1"));
  _modem -> waitResponse(1000L, GF("OK"));  
  _modem -> sendAT(GF("+CLIP=1"));
  _modem -> waitResponse(1000L, GF("OK"));  // Auto Phone Number Detection
  _modem -> sendAT(GF("+DDET=1,0,0"));  // DTMF Keyboard ON
  _modem -> waitResponse(1000L, GF("OK"));
}

void SystemController::blinkLed(void) {

}

void SystemController::setMakeShiftPower(boolean isMakeshift) {
  _config -> isMakeshitPower = isMakeshift;
  resetMakeshiftTimer();

  if(isMakeshift)
    relayState = _config -> isLowLevelRelay;
  else {
    digitalWrite(
      _config -> relayPin,
      _config -> isLowLevelRelay
    );
  }

}

void SystemController::setNextRelayState(void) {    
  if(_config -> isMakeshitPower) {
    digitalWrite(
      _config -> relayPin,
      _config -> isOnByDefaultMakeshift ?  (_config -> isLowLevelRelay) : (!_config -> isLowLevelRelay)
    );
    makeshiftPowerTimer = millis() + _config -> makeShiftPowerDuration*1000;
    timerState = ACTIVE;
  }
  else {
    relayState = !relayState;
    digitalWrite(_config -> relayPin, relayState);  
  }
}

void SystemController::resetMakeshiftTimer(void) {  
  digitalWrite(
    _config -> relayPin,
    _config -> isOnByDefaultMakeshift ? (!_config -> isLowLevelRelay) : (_config -> isLowLevelRelay)
  );
  makeshiftPowerTimer = 0;
  timerState = DISABLED;
}

void SystemController::loop(void) {      
  if(_instance -> timerState == ACTIVE && millis() > _instance -> makeshiftPowerTimer)
    _instance -> resetMakeshiftTimer();
  
  if(_instance -> _config -> isMakeshitPower) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }

  if(_instance -> _modem -> waitResponse(10, GF("RING")) == 1) {            
    String buffer = _instance -> _modem -> stream.readString();
    buffer = buffer.substring(0, buffer.lastIndexOf("\nOK"));    
    uint8_t command_index = buffer.indexOf("+CLIP: \"") + 8;
    if(command_index > 7) {
      String phoneNumber = buffer.substring(command_index, buffer.indexOf("\",", command_index));
      if(_instance -> isWhiteListed(phoneNumber))
        _instance -> setNextRelayState();
      _instance -> _modem -> sendAT(GF("H"));
    }
  }
}

boolean SystemController::isWhiteListed(const String& phone) {    
  for(uint8_t index = 0; index < _config -> phoneWhitelistQuantity; index++) {
    if(!strcmp(_config -> phoneWhitelist[index], phone.c_str()))
      return true;
  }  
  return false;
}

SystemController::~SystemController() {}