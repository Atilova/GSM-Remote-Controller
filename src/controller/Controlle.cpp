#include "Arduino.h"
#include "Controller.h"


SystemController::SystemController(TinyGsm& modem, controllerConfig& _config) {    
  this -> _modem = &modem;   
  this -> _config = &_config;
}

void SystemController::loop(void) {
  if(_instance -> timerState == ACTIVE && millis() > _instance -> makeshiftPowerTimer)
    _instance -> resetMakeshiftTimer();

  if(_instance -> _modem -> waitResponse(100, GF("RING")) == 1) {            
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

void SystemController::addInterrupt(void) {
  isButtunPressed = false;
  attachInterrupt(
    digitalPinToInterrupt(_config -> swapModePin), 
    SystemController::swapModeISR,
    FALLING
  );
}

void SystemController::blinkLed(void) {  
  if(_instance -> isButtunPressed) {    
    if(millis() > _instance -> lastTimePressed) {
      _instance -> setMakeShiftPower(!_instance -> _config -> isMakeshitPower);
      _instance -> addInterrupt();
    } 
    else if(digitalRead(_instance -> _config -> swapModePin) == HIGH)
      _instance -> addInterrupt();    
  } 

  if(!_instance -> _config -> isMakeshitPower)
    return;
  
  if(millis() > _instance -> lastTimeBlinked) {
    _instance -> lastTimeBlinked = millis() + 300;
    if(digitalRead(LED_BUILTIN))
      digitalWrite(LED_BUILTIN, LOW);
    else
      digitalWrite(LED_BUILTIN, HIGH);
  }
}

void SystemController::swapModeISR(void) {
  _instance -> isButtunPressed = true;
  _instance -> lastTimePressed = millis() + 700;
  detachInterrupt(digitalPinToInterrupt(_instance -> _config -> swapModePin));
}

void SystemController::begin(void) {  
  pinMode(_config -> relayPin, OUTPUT);
  pinMode(_config -> swapModePin, INPUT_PULLUP);

  setRelayState(
    _config -> isMakeshitPower && _config -> isOnByDefaultMakeshift 
      ? HIGH
      : LOW
  );
  resetModem();

  _instance = this;
  ThreadHandler::getInstance()->enableThreadExecution();  
  addInterrupt();
}

void SystemController::setRelayState(boolean nextRelayState) {
  relayState = (
    nextRelayState
      ? !_config -> isLowLevelRelay 
      : _config -> isLowLevelRelay
  );
    
  digitalWrite(
    _config -> relayPin,
    relayState
  );
};

void SystemController::resetModem(void) {
  digitalWrite(_config -> resetPin, HIGH);
  pinMode(_config -> resetPin, OUTPUT);
  delay(100);
  digitalWrite(_config -> resetPin, LOW);
  delay(200);
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

void SystemController::setMakeShiftPower(boolean isMakeshift) {
  _config -> isMakeshitPower = isMakeshift;
  resetMakeshiftTimer();

  if(isMakeshift)
    relayState = _config -> isLowLevelRelay;
  else {
    digitalWrite(LED_BUILTIN, LOW);
    setRelayState(LOW);  
  }
}

void SystemController::setNextRelayState(void) {    
  if(_config -> isMakeshitPower) {
    setRelayState(_config -> isOnByDefaultMakeshift ? LOW : HIGH);
    makeshiftPowerTimer = millis() + _config -> makeShiftPowerDuration*1000;
    timerState = ACTIVE;
  }
  else
    setRelayState(!relayState);      
}

void SystemController::resetMakeshiftTimer(void) {  
  setRelayState(_config -> isOnByDefaultMakeshift ? HIGH : LOW);
  makeshiftPowerTimer = 0;
  timerState = DISABLED;
}

boolean SystemController::isWhiteListed(const String& phone) {    
  for(uint8_t index = 0; index < _config -> phoneWhitelistQuantity; index++) {
    if(!strcmp(_config -> phoneWhitelist[index], phone.c_str()))
      return true;
  }  
  return false;
}

SystemController::~SystemController() {}