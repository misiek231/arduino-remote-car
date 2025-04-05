#include <Servo.h>
#include <SoftwareSerial.h>

#define TIMEOUT 20000 // mS
#define SOFT_SERIAL 0

Servo steeringServo;
Servo engineServo;
Servo gearServo;

#if SOFT_SERIAL == 1
SoftwareSerial espSerial(2, 3); // RX, TX
#endif

int steeringDefaultPos = 90; // Initial steering position (going forward)
int engineDefaultSpeed = 100; // Initial engine speed (stopped)
int gearDefaultPos = 90; // Initial engine speed (stopped)
String engineCurrValue = "c"; // Initial engine speed (stopped)

void setup() {
  Serial.begin(9600);

#if SOFT_SERIAL == 1
  espSerial.begin(9600); // Start serial communication with the ESP-01s
#endif

#if SOFT_SERIAL == 0
  steeringServo.attach(9); // Attach the steering servo to pin 10
  engineServo.attach(10); // Attach the engine servo to pin 9
  gearServo.attach(11); // Attach the steering servo to pin 10

  steeringServo.write(steeringDefaultPos); // Set initial steering position
  gearServo.write(gearDefaultPos); // Set initial steering position
  engineServo.write(engineDefaultSpeed); // Set initial engine speed
#endif

#if SOFT_SERIAL == 1
  Serial.println("Begin");
#endif

  SendCommand("AT+RST", "Ready");
  delay(5000);
  SendCommand("AT", "OK");
  SendCommand("AT+CWMODE=1", "OK");
  SendCommand("AT+CWJAP=\"UPC2558138\",\"Mpuff7bpjeku\"", "OK");
  SendCommand("AT+CIFSR", "OK");
  SendCommand("AT+CIPMUX=1", "OK");
  SendCommand("AT+CIPSTART=0,\"UDP\",\"0.0.0.0\",4210,4210,0", "OK");

#if SOFT_SERIAL == 1
  Serial.println("Seems ok!");
#endif

}

void loop() {
  static String IncomingString = "";
  boolean StringReady = false;

#if SOFT_SERIAL == 1
  Stream& serialStream = espSerial;  
#else
  Stream& serialStream = Serial;  
#endif

  while (serialStream.available()) {
    char incomingChar = serialStream.read();
    if (incomingChar == '\n') { // Assuming the end of the message is marked by a newline character
      if (IncomingString.length() > 0) { // Only process non-empty strings
        StringReady = true;
      }
      break;
    } else if (incomingChar != '\r') { // Ignore carriage return characters
      IncomingString += incomingChar;
    }
  }


  if (!StringReady) return;

  int startIndex = IncomingString.indexOf(':') + 1;
  String input = IncomingString.substring(startIndex);

  // Split into steer, engine, and gear
  int firstColon = input.indexOf(':');
  int secondColon = input.indexOf(':', firstColon + 1);

  String steerValueS = input.substring(0, firstColon);
  int steerValue = steerValueS.toInt();
  String engineValue = input.substring(firstColon + 1, secondColon);
  String gearValue = input.substring(secondColon + 1);

 // Serial.println(gearValue);

  // Parse steer to int
  
#if SOFT_SERIAL == 1
  Serial.print("steerValue: ");
  Serial.println(steerValue);

  Serial.print("engineValue: ");
  Serial.println(engineValue);

  
  Serial.print("gear value: ");
  Serial.println(gearValue);
#endif

  if(steerValue < 40) steerValue = 40;
  if(steerValue > 140) steerValue = 140;

  smoothMoveServo(steeringServo, steerValue);

  // steeringServo.write(steerValue);

  if (engineValue == "w") {
    engineCurrValue = "w";
    engineServo.write(engineDefaultSpeed + 50);
  } else if (engineValue == "s") {
    engineCurrValue = "s";
    engineServo.write(engineDefaultSpeed - 20);
  } else if (engineValue == "c") {
    if(engineCurrValue == "w"){
      engineServo.write(engineDefaultSpeed - 0);
    } else if(engineCurrValue == "s"){
      engineServo.write(engineDefaultSpeed + 10);
    }
  }

  if (gearValue == "v") {
    gearServo.write(gearDefaultPos + 50);
  } else if (gearValue == "b") {
    gearServo.write(gearDefaultPos - 50);
  } else {
    gearServo.write(gearDefaultPos);
  }

  IncomingString = ""; // Clear the string for the next message
  StringReady = false;
}

void smoothMoveServo(Servo& servo, int targetPosition) {
  int currentPosition = servo.read();
  while (currentPosition != targetPosition) {
    if (currentPosition < targetPosition) {
      currentPosition++;
    } else {
      currentPosition--;
    }
    servo.write(currentPosition);
    delay(1); // Adjust delay as needed
  }
}

boolean SendCommand(String cmd, String ack) {

#if SOFT_SERIAL == 1
  espSerial.println(cmd); // Send "AT+" command to module
#endif

  Serial.println(cmd);
  return echoFind(ack); // Check for ack string
}

boolean echoFind(String keyword) {
#if SOFT_SERIAL == 1
  Stream& serialStream = espSerial;  
#else
  Stream& serialStream = Serial;  
#endif

  byte current_char = 0;
  byte keyword_length = keyword.length();
  long deadline = millis() + TIMEOUT;
  while (millis() < deadline) {
    if (serialStream.available()) {
      char ch = serialStream.read();
      if (ch == keyword[current_char]) {
        if (++current_char == keyword_length) {

#if SOFT_SERIAL == 1
          Serial.println(keyword);
#endif
          return true;
        }
      }
    }
  }
  return false; // Timed out
}