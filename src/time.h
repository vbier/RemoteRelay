#ifndef _TIME_H
#define _TIME_H 1

#include "Arduino.h"

void time_setup();
void time_loop();

String getFormattedTime();
time_t localTime();

#endif