#include <Servo.h>
#include <SoftwareSerial.h>
#include "Config.h"

#if USE_SOFT_SERIAL
SoftwareSerial espSerial(2, 3); // RX, TX
#endif

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

// === Setup ===
void setup() {
  setupConfig();

  Serial.begin(9600);

#if USE_SOFT_SERIAL
  espSerial.begin(9600);
  Serial.println("Begin");
#endif

#if !USE_SOFT_SERIAL
  steeringServo.attach(STEERING_SERVO_PIN);
  engineServo.attach(ENGINE_SERVO_PIN);
  gearServo.attach(GEAR_SERVO_PIN);

  steeringServo.write(steeringDefaultPos);
  engineServo.write(engineDefaultSpeed);
  gearServo.write(gearDefaultPos);
#endif

  setupWiFi();
}

// === Loop ===
void loop() {
  static String incomingString = "";
  static bool stringReady = false;

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

  if (stringReady) {
#if USE_SOFT_SERIAL
    Serial.println(incomingString);
#endif
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

#if USE_SOFT_SERIAL
  Serial.print("Steer: "); Serial.println(steer);
  Serial.print("Engine: "); Serial.println(engine);
  Serial.print("Gear: "); Serial.println(gear);
#endif

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

// === Serial selector ===
Stream& getSerial() {
#if USE_SOFT_SERIAL
  return espSerial;
#else
  return Serial;
#endif
}

// === WiFi setup ===
void setupWiFi() {

    // Check if already connected to the correct network
  if (!SendCommand("AT", "OK")) {
    SendCommand("AT+RST", "Ready");
    delay(500);
    SendCommand("AT", "OK");
  }

  // Check if already connected
  if (!SendCommand("AT+CWJAP?", WIFI_SSID)) {
    SendCommand("AT+CWMODE=1", "OK");
    SendCommand("AT+CWJAP=\"" + String(WIFI_SSID) + "\",\"" + String(WIFI_PASSWORD) + "\"", "OK");
  }

  SendCommand("AT+CIFSR", "OK"); // returns current ip

  if(!SendCommand("AT+CIPSTATUS", "+CIPSTATUS:0")) {
    SendCommand("AT+CIPMUX=0", "OK");
    SendCommand("AT+CIPSTART=\"UDP\",\"0.0.0.0\",4210,4210,0", "OK");
  }

#if USE_SOFT_SERIAL
  Serial.println("WiFi setup complete.");
#endif
}

// === AT command helpers ===
bool SendCommand(const String& cmd, const String& ack) {
#if USE_SOFT_SERIAL
  espSerial.println(cmd);
#endif
  Serial.println(cmd);
  return waitForAck(ack);
}

bool waitForAck(const String& keyword) {
  Stream& serialStream = getSerial();
  String response = "";
  long deadline = millis() + ESP_COMMAND_TIMEOUT;

  while (millis() < deadline) {
    while (serialStream.available()) {
      char ch = serialStream.read();
      response += ch;

      // Optional debug
       // Serial.print(ch);

      if (response.indexOf(keyword) != -1 || response.indexOf("ALREADY CONNECTED") != -1) {
#if USE_SOFT_SERIAL
        Serial.println("Matched: " + keyword);
#endif
        return true;
      }
    }
  }
#if USE_SOFT_SERIAL
  Serial.println("Timeout or no match: " + keyword + " recived:" + response);
#endif
  return false;
}

