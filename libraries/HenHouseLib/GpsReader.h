#ifndef GpsReader_h
#define GpsReader_h

#include "arduino.h"
#include "StateReader.h"
#include "SoftwareSerial.h"
#include "TinyGPS.h"

class GpsReader : public StateReader {

public:
    GpsReader();
    
    virtual State ReadState(State state);
};

#endif