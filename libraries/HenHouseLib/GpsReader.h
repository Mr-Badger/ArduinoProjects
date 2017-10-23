#ifndef GpsReader_h
#define GpsReader_h

#include "arduino.h"
#include "StateReader.h"
#include "SoftwareSerial.h"
#include "TinyGPS.h"

class GpsReader : public StateReader {


private:
	SoftwareSerial mySerial; // 10 is 9 and 11 is 10 ???
	TinyGPS gps;

public:
    GpsReader();
    
    virtual State ReadState(State state);
};

#endif