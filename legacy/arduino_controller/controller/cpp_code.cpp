#include <Servo.h>
#include <SoftwareSerial.h>
#include <String.h>
#include "cpp_code.h"
#include "Config.h"
#include "common.h"
#include "wifi_connector.h"

// === Servo Instances ===
Servo steeringServo;
Servo engineServo;
Servo gearServo;

// === Engine easing state machine ===
enum EngineState {
  ENGINE_IDLE,
  ENGINE_PUSHING,
  ENGINE_RETRACTING
};

struct EngineEase {
  EngineState state = ENGINE_IDLE;
  unsigned long stateStartTime = 0;
  char currentCommand = 'c';
};

EngineEase engineEase;
const int ENGINE_PUSH_DURATION = 100; // milliseconds

unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 200; // ms

void processInput(const String& message);
void updateEngineState(char command);
void updateEngineEase();
void updateGearState(char command);
void updateSteerState(Servo& servo, int target);
void setupServos();
void updateAutostop();
bool readInput(String* input);

// === Setup ===
void setup_cpp() {
  
  setupConfig();
  setupSerial();
  setupWifi();
  setupServos();
}

// === Loop ===
void loop_cpp() {

  bool connected = updateWiFiSetup();

  if(!connected) return;

  String input = "";

  bool stringReady = readInput(&input);

  if (stringReady) {
    processInput(input);
  }

  updateEngineEase(); // Non-blocking servo retract logic
  updateAutostop();
}

void updateAutostop(){
    
  if (millis() - lastCommandTime < COMMAND_TIMEOUT) return;
  // Auto-stop if no command for over 200ms
  updateEngineState('c'); // Stop the engine
}

bool readInput(String* input){
  static bool stringReady = false;

  if(stringReady == true){
    stringReady = false;
    *input = "";
  }

  Stream& serialStream = getSerial();

  while (serialStream.available()) {
    
    char c = serialStream.read();

    if (c == '\r') continue;

    if (c == '\n') {
      stringReady = (*input).length() > 0;
      break;
    } 

    *input += c;
  }

  if(!stringReady) return false;

  return true;
}

// === Process incoming command ===
void processInput(const String& message) {

  debugLog("RX: " + message);
  lastCommandTime = millis();  // Update timestamp on new command

  // Format: [ID]:[steer]:[engine]:[gear]
  // Example: 110:90:w:v
  int startIndex = message.indexOf(':') + 1;
  String input = message.substring(startIndex);

  int firstColon = input.indexOf(':');
  int secondColon = input.indexOf(':', firstColon + 1);

  int steer = input.substring(0, firstColon).toInt();
  steer = constrain(steer, STEERING_MIN, STEERING_MAX);

  char engine = input.charAt(firstColon + 1);
  char gear = input.charAt(secondColon + 1);

  debugLog("Steer: " + String(steer));
  debugLog("Engine: " + String(engine));
  debugLog("Gear: " + String(gear));

  updateSteerState(steeringServo, steer);
  updateEngineState(engine);
  updateGearState(gear);
}

// === Update engine with push-and-retract easing ===
void updateEngineState(char command) {
  if (command == engineEase.currentCommand) return;
  engineEase.currentCommand = command;

  if (command == 'w' || command == 's') {
    int pushOffset = engineOffsetTable[command];
    engineServo.write(engineDefaultSpeed + pushOffset);

    engineEase.state = ENGINE_PUSHING;
    engineEase.stateStartTime = millis();
  } else { // 'c' or unknown
    engineServo.write(engineDefaultSpeed);
    engineEase.state = ENGINE_IDLE;
  }
}

void updateEngineEase() {
  if (engineEase.state != ENGINE_PUSHING) return;
  if (millis() - engineEase.stateStartTime < ENGINE_PUSH_DURATION) return;

  char cmd = engineEase.currentCommand;
  int retractOffset = engineEaseTargetOffsetTable[cmd];

  engineServo.write(engineDefaultSpeed + retractOffset);
  engineEase.state = ENGINE_RETRACTING;
}

// === Update gear servo ===
void updateGearState(char command) {
  int offset = gearOffsetTable[command];
  gearServo.write(gearDefaultPos + offset);
}

// === Smoothly move servo (used for steering) ===
void updateSteerState(Servo& servo, int target) {
  int current = servo.read();
  while (current != target) {
    current += (current < target) ? 1 : -1;
    servo.write(current);
    delay(1); // Tune for smoothness
  }
}

void setupServos(){
#if !USE_SOFT_SERIAL
  steeringServo.attach(STEERING_SERVO_PIN);
  engineServo.attach(ENGINE_SERVO_PIN);
  gearServo.attach(GEAR_SERVO_PIN);

  steeringServo.write(steeringDefaultPos);
  engineServo.write(engineDefaultSpeed);
  gearServo.write(gearDefaultPos);
#endif
}


