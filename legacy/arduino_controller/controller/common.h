#ifndef COMMON_H
#define COMMON_H
#include <Arduino.h>

void setupSerial();
Stream& getSerial();
void debugLog(const String& message);


#endif // COMMON_H