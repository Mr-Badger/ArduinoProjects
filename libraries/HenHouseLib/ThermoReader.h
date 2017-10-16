#ifndef ThermoReader_h
#define ThermoReader_h

#include "arduino.h"
#include "StateReader.h"

class ThermoReader : public StateReader 
{
private:
    const int THERMO_PIN = A3;
public: 
    virtual State ReadState(State state);
};

#endif