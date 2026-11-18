// cpp_code.h

#ifndef _CPP_CODE_h
#define _CPP_CODE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

void setup_cpp();
void loop_cpp();

#endif

