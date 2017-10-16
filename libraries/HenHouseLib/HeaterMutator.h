#ifndef HeaterMutator_h
#define HeaterMutator_h

#include "arduino.h"
#include "Logger.h"
#include "StateMutator.h"

class HeaterMutator : public StateMutator
{
private:
    void TurnOffHeater();
    void TurnOnHeater();

    Logger* _logger;

    const int HEATER_PIN = 11;

public:
    virtual State MutateState(State state);
    HeaterMutator(Logger* logger);
};

#endif