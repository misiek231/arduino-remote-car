// wifi_connector.h

#ifndef _WIFI_CONNECTOR_h
#define _WIFI_CONNECTOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "common.h"

//bool waitForAck(const String& keyword);
//bool SendCommand(const String& cmd, const String& ack);
//void setupWiFi();
bool updateWiFiSetup();

void setupWifi();
void updateWifiLED();

#endif

