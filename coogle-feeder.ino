#include <CoogleIOT.h>
#include <AccelStepper.h>

#ifndef SERIAL_BAUD
#define SERIAL_BAUD 115200
#endif

#ifndef STEPPER_SPEED
#define STEPPER_SPEED 1000
#endif

#ifndef STEPPER_ACCEL
#define STEPPER_ACCEL 500
#endif

#define ACTION_TOPIC "/feeder/saltwater/1"

CoogleIOT *iot;
PubSubClient *mqtt;

AccelStepper stepper(AccelStepper::DRIVER, D1, D2);

char msg[150];
int turnsToExecute = 0;

void setup() {

  iot = new CoogleIOT(LED_BUILTIN);

  iot->enableSerial(SERIAL_BAUD);
  iot->initialize();
   
  iot->info("CoogleFeeder Initializing...");
  iot->info("-=-=-=-=--=--=-=-=-=-=-=-=-=-=-=-=-=-");
  iot->logPrintf(INFO, "MQTT Action Topic: %s", ACTION_TOPIC);
  iot->info("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");

  stepper.setMaxSpeed(STEPPER_SPEED);
  stepper.setAcceleration(STEPPER_ACCEL);
  stepper.setSpeed(STEPPER_SPEED);
    
  iot->info("");

  if(iot->mqttActive()) {
      mqtt = iot->getMQTTClient();
    
      mqtt->setCallback(mqttCallback);

      mqtt->subscribe(ACTION_TOPIC);
           
      iot->info("CoogleFeeder Initialized!");
  } else {
    iot->error("Initialization failure, invalid MQTT Server connection.");
  }
   
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{ 
  String input((char *)payload);
  
  turnsToExecute = input.toInt();
  
  iot->logPrintf(DEBUG, "MQTT Callback Triggered. Topic: %s\n", topic);
  iot->logPrintf(DEBUG, "Turns to execute: %d", turnsToExecute);

  iot->info("Action Complete!");
}

void loop() {
  iot->loop();
  
  if(stepper.distanceToGo() == 0) {
    if(turnsToExecute > 0) {
      stepper.moveTo(stepper.currentPosition() + (1600 * turnsToExecute));
      turnsToExecute = 0;

       
    }
  }
  
  stepper.run();
}