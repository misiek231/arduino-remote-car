#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include "common.h"

bool waitForAck(const String& keyword);
bool SendCommand(const String& cmd, const String& ack);
void setupWiFi();
bool updateWiFiSetup();

#endif // WIFI_H