#ifndef LightMutator_h
#define LightMutator_h

#include "arduino.h"
#include "Logger.h"
#include "StateMutator.h"

class LightMutator : public StateMutator
{
private:
    void TurnOffLight();
    void TurnOnLight();

    Logger* _logger;

    const int LIGHT_PIN = 12;    

public:
    virtual State MutateState(State state);
    LightMutator(Logger* logger);
};

#endif