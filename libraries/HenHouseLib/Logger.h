#ifndef Logger_h
#define Logger_h

#include "arduino.h"
#include "State.h"

class Logger {
public:
    Logger();
    void Debug(char* message);
    void SetTime(long time);
};

#endif