#include <CoogleIOT.h>
#include <AccelStepper.h>

#ifndef SERIAL_BAUD
#define SERIAL_BAUD 115200
#endif

#define ACTION_TOPIC "/feeder/saltwater/1"

#define USE_NEMA17
//#define USE_28BYJ

#ifdef USE_28BYJ
#define MOTOR_28BYJ_PIN1 5
#define MOTOR_28BYJ_PIN2 4
#define MOTOR_28BYJ_PIN3 12
#define MOTOR_28BYJ_PIN4 13
#define MOTOR_TYPE AccelStepper::HALF4WIRE
#define HALF_STEP 16
#define STEPPER_SPEED 200
#define STEPPER_ACCEL 50
#endif

#ifdef USE_NEMA17
#define NEMA_STEP_PIN D1
#define NEMA_DIR_PIN D2
#define HALF_STEP 800
#define MOTOR_TYPE AccelStepper::DRIVER
#define STEPPER_SPEED 1000
#define STEPPER_ACCEL 500
#endif

CoogleIOT *iot;
PubSubClient *mqtt;

AccelStepper *stepper;

char msg[150];
int turnsToExecute = 0;

void setup() {

#ifdef USE_NEMA17
  stepper = new AccelStepper(MOTOR_TYPE, NEMA_STEP_PIN, NEMA_DIR_PIN);
#endif

#ifdef USE_28BYJ
  stepper = new AccelStepper(MOTOR_TYPE, MOTOR_28BYJ_PIN1, MOTOR_28BYJ_PIN2, MOTOR_28BYJ_PIN3, MOTOR_28BYJ_PIN4);
#endif

  iot = new CoogleIOT(LED_BUILTIN);

  iot->enableSerial(SERIAL_BAUD);
  iot->initialize();
   
  iot->info("CoogleFeeder Initializing...");
  iot->info("-=-=-=-=--=--=-=-=-=-=-=-=-=-=-=-=-=-");
  iot->logPrintf(INFO, "MQTT Action Topic: %s", ACTION_TOPIC);
  iot->info("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");

  stepper->setMaxSpeed(STEPPER_SPEED);
  stepper->setAcceleration(STEPPER_ACCEL);
  stepper->setSpeed(STEPPER_SPEED);
    
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
  
  if(stepper->distanceToGo() == 0) {
    if(turnsToExecute > 0) {
      stepper->moveTo(stepper->currentPosition() + ((HALF_STEP / 2) * turnsToExecute));
      turnsToExecute = 0;

       
    }
  }
  
  stepper->run();
}
