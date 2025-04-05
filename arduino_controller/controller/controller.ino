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

  if (!stringReady) return;

  processInput(incomingString);
  incomingString = "";
  stringReady = false;
}

// === Process incoming command ===
void processInput(const String& message) {

  // incoming message example: 110:w:v
  int startIndex = message.indexOf(':') + 1;
  String input = message.substring(startIndex);

  int firstColon = input.indexOf(':');
  int secondColon = input.indexOf(':', firstColon + 1);

  int steer = input.substring(0, firstColon).toInt();
  steer = constrain(steer, 40, 140);

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

void updateEngine(char command) {
  int offset = engineOffsetTable[command];
  engineServo.write(engineDefaultSpeed + offset);
}

void updateGear(char command) {
  int offset = gearOffsetTable[command];
  gearServo.write(gearDefaultPos + offset);
}

// === Smoothly move servo ===
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
  SendCommand("AT+RST", "Ready");
  delay(5000);
  SendCommand("AT", "OK");
  SendCommand("AT+CWMODE=1", "OK");
  SendCommand("AT+CWJAP=\"UPC2558138\",\"Mpuff7bpjeku\"", "OK");
  SendCommand("AT+CIFSR", "OK");
  SendCommand("AT+CIPMUX=1", "OK");
  SendCommand("AT+CIPSTART=0,\"UDP\",\"0.0.0.0\",4210,4210,0", "OK");

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
  byte matched = 0;
  long deadline = millis() + ESP_COMMAND_TIMEOUT;

  while (millis() < deadline) {
    if (serialStream.available()) {
      char ch = serialStream.read();
      if (ch == keyword[matched]) {
        if (++matched == keyword.length()) {
#if USE_SOFT_SERIAL
          Serial.println(keyword);
#endif
          return true;
        }
      } else {
        matched = 0; // Reset on mismatch
      }
    }
  }
  return false;
}
