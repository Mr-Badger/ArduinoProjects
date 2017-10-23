#ifndef InternetReader_h
#define InternetReader_h

#include "arduino.h"
#include "StateReader.h"
#include "SoftwareSerial.h"

class InternetReader : public StateReader {

private:
	SoftwareSerial wifiSerial;	

public:
	InternetReader();	
	String WifiData(String command, const int timeout, boolean debug);
    virtual State ReadState(State state);
};

#endif