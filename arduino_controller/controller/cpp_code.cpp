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
void updateEngine(char command);
void updateEngineEase();
void updateGear(char command);
void smoothMoveServo(Servo& servo, int target);
void setupServos();

// === Setup ===
void setup_cpp() {
  setupConfig();
  setupSerial();
  setupServos();
}

// === Loop ===
void loop_cpp() {

  bool connected = updateWiFiSetup();  

  if(!connected) return;



  if (stringReady) {
    debugLog(incomingString);
    lastCommandTime = millis();  // Update timestamp on new command
    processInput(incomingString);
    incomingString = "";
    stringReady = false;
  }

  updateEngineEase(); // Non-blocking servo retract logic

    // Auto-stop if no command for over 200ms
  if (millis() - lastCommandTime > COMMAND_TIMEOUT) {
      updateEngine('c'); // Stop the engine
  }
}

void readInput(String* input){
    static String incomingString = "";
  static bool stringReady = false;


  if(stringReady == true){
    stringReady = false;
    incomingString = "";
  }

  Stream& serialStream = getSerial();

  while (serialStream.available()) {
    char c = serialStream.read();
    if (c == '\n') {
      stringReady = incomingString.length() > 0;
      break;
    } else if (c != '\r') {
      incomingString += c;
    }
  }

  if(!stringReady) return false;

  input* = incomingString;
  return true;
}

// === Process incoming command ===
void processInput(const String& message) {

  // incoming message example: 110:w:v
  int startIndex = message.indexOf(':') + 1;
  String input = message.substring(startIndex);

  int firstColon = input.indexOf(':');
  int secondColon = input.indexOf(':', firstColon + 1);

  int steer = input.substring(0, firstColon).toInt();
  steer = constrain(steer, STEERING_MIN, STEERING_MAX);

  char engine = input.charAt(firstColon + 1);
  char gear = input.charAt(secondColon + 1);

  debugLog("Steer: "); debugLog(String(steer));
  debugLog("Engine: "); debugLog(String(engine));
  debugLog("Gear: "); debugLog(String(gear));

  smoothMoveServo(steeringServo, steer);
  updateEngine(engine);
  updateGear(gear);
}

// === Update engine with push-and-retract easing ===
void updateEngine(char command) {
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
void updateGear(char command) {
  int offset = gearOffsetTable[command];
  gearServo.write(gearDefaultPos + offset);
}

// === Smoothly move servo (used for steering) ===
void smoothMoveServo(Servo& servo, int target) {
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


