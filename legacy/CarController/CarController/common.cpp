// 
// 
// 

#include "common.h"


#if USE_SOFT_SERIAL
SoftwareSerial espSerial(2, 3); // RX, TX
#endif

void setupSerial() {

    Serial.begin(9600);

#if USE_SOFT_SERIAL
    Serial.println("Debug log enabled and initialized");
    espSerial.begin(9600);
#endif
}

Stream& getSerial() {
#if USE_SOFT_SERIAL
    return espSerial;
#else
    return Serial;
#endif
}

void debugLog(const String& message) {
#if USE_SOFT_SERIAL
    Serial.println(message);
#endif
}