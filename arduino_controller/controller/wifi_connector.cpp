#include <Arduino.h>
#include "wifi_connector.h"
#include "Config.h"
#include "common.h"

struct ATCommand {
  String command;
  String expectedReply;
  unsigned long timeout;
  unsigned long startTime;
  byte matched = 0;
  bool active = false;
};

ATCommand currentAT;

enum WiFiSetupState {
  WIFI_RESETTING,
  WIFI_WAIT_READY,
  WIFI_SET_MODE,
  WIFI_CONNECTING,
  WIFI_GET_IP,
  WIFI_SET_UDP,
  WIFI_DONE,
  WIFI_IDLE,
  WIFI_FAILED
};

WiFiSetupState wifiState = WIFI_RESETTING;
unsigned long wifiRetryTimer = 0;
bool wifiConnected = false;

void startATCommand(const String& cmd, const String& expected, unsigned long timeoutMs = 2000) {
  // Start new command
  currentAT.command = cmd;
  currentAT.expectedReply = expected;
  currentAT.timeout = timeoutMs;
  currentAT.startTime = millis();
  currentAT.matched = 0;
  currentAT.active = true;

  Stream& serialStream = getSerial();

  serialStream.println(cmd);
  debugLog(">>> " + cmd);
}

bool processATResponse() {
  if (!currentAT.active) return false;

  Stream& serial = getSerial();

  while (serial.available()) {
    char ch = serial.read();

    if (ch == currentAT.expectedReply[currentAT.matched]) {
      currentAT.matched++;
      if (currentAT.matched == currentAT.expectedReply.length()) {
        currentAT.active = false;
        
        debugLog("*** Match success");
        return true;
      }
    } else {
      currentAT.matched = 0;
    }
  }

  // Check for timeout
  if (millis() - currentAT.startTime > currentAT.timeout) {
    currentAT.active = false;
#if USE_SOFT_SERIAL
    Serial.println("*** Match timeout");
#endif
    return false;
  }

  // Still waiting
  return false;
}

bool updateWiFiSetup() {
  if (currentAT.active && !processATResponse()) {
    return wifiConnected; // Wait for current command to finish
  }

  switch (wifiState) {
    case WIFI_RESETTING:
      startATCommand("AT+RST", "Ready", 5000);
      wifiState = WIFI_WAIT_READY;
      break;

    case WIFI_WAIT_READY:
      startATCommand("AT", "OK");
      wifiState = WIFI_SET_MODE;
      break;

    case WIFI_SET_MODE:
      startATCommand("AT+CWMODE=1", "OK");
      wifiState = WIFI_CONNECTING;
      break;

    case WIFI_CONNECTING:
      startATCommand("AT+CWJAP=\"" + String(WIFI_SSID) + "\",\"" + String(WIFI_PASSWORD) + "\"", "OK");
      wifiState = WIFI_GET_IP;
      break;

    case WIFI_GET_IP:
      startATCommand("AT+CIFSR", "OK");
      wifiState = WIFI_SET_UDP;
      break;

    case WIFI_SET_UDP:
      startATCommand("AT+CIPMUX=0", "OK");
      wifiState = WIFI_DONE;
      break;

    case WIFI_DONE:
      startATCommand("AT+CIPSTART=\"UDP\",\"0.0.0.0\",4210,4210,0", "OK");
      wifiConnected = true;
      wifiState = WIFI_IDLE;
      break;

    case WIFI_IDLE:
      // Stay connected
      break;

    case WIFI_FAILED:
      wifiConnected = false;
      if (millis() - wifiRetryTimer > 5000) {
        wifiState = WIFI_RESETTING;
      }
      break;
  }

  return wifiConnected;
}












// bool waitForAck(const String& keyword) {
//   Stream& serialStream = getSerial();
//   String response = "";
//   long deadline = millis() + ESP_COMMAND_TIMEOUT;

//   while (millis() < deadline) {
//     while (serialStream.available()) {
//       char ch = serialStream.read();
//       response += ch;

//       // Optional debug
//        // Serial.print(ch);

//       if (response.indexOf(keyword) != -1 || response.indexOf("ALREADY CONNECTED") != -1) {
//         debugLog("Matched: " + keyword);
//         return true;
//       }
//     }
//   }

//   debugLog("Timeout or no match: " + keyword + " recived:" + response);
//   return false;
// }

// // === AT command helpers ===
// bool SendCommand(const String& cmd, const String& ack) {

//   Stream& serialStream = getSerial();
//   serialStream.println(cmd);
//   debugLog(cmd);
//   return waitForAck(ack);
// }

// void setupWiFi() {

//     // Check if already connected to the correct network
//   if (!SendCommand("AT", "OK")) {
//     SendCommand("AT+RST", "Ready");
//     delay(500);
//     SendCommand("AT", "OK");
//   }

//   // Check if already connected
//   if (!SendCommand("AT+CWJAP?", WIFI_SSID)) {
//     SendCommand("AT+CWMODE=1", "OK");
//     SendCommand("AT+CWJAP=\"" + String(WIFI_SSID) + "\",\"" + String(WIFI_PASSWORD) + "\"", "OK");
//   }

//   SendCommand("AT+CIFSR", "OK"); // returns current ip

//   if(!SendCommand("AT+CIPSTATUS", "+CIPSTATUS:0")) {
//     SendCommand("AT+CIPMUX=0", "OK");
//     SendCommand("AT+CIPSTART=\"UDP\",\"0.0.0.0\",4210,4210,0", "OK");
//   }

//   debugLog("WiFi setup complete.");
// }