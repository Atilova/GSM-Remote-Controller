#include <SoftwareSerial.h> 
#include "controller/Controller.h"

#ifndef CONFIG_H_
  #define CONFIG_H_

  SoftwareSerial modemIO(DD3, DD4);  // Rx TX
  #define TINY_GSM_MODEM_SIM800
  #define SWAP_MODE_PIN DD2
  #define RELAY_PIN 12
  #define GSM_IO modemIO
  #define GSM_RESET_PIN DD7

  const char* whitelist[] = {
    "+380679192808",
    "+380675794863",
    "+380977240401"
  };

  controllerConfig systemConfig = {
    whitelist,
    sizeof(whitelist) / sizeof(char*),
    GSM_RESET_PIN,
    RELAY_PIN,
    SWAP_MODE_PIN,
    false,
    true,
    10,
    false
  };
#endif