#include <Arduino.h>
#include <Config.h>
#include <controller/Controller.h>
#include <TinyGSM.h>
#include <ThreadHandler.h>


SET_THREAD_HANDLER_TICK(1000);
THREAD_HANDLER(InterruptTimer::getInstance());

TinyGsm modem(modemIO);
SystemController controller(modem, systemConfig);
SystemController* SystemController::_instance = NULL;


void setup() {
  Serial.begin(115200);  
  // GSM_IO.begin(9600);
  // controller.begin();
}

void loop() {
  // controller.loop();
  delay(2000);
  Serial.println("<<LOOPING>>");
}