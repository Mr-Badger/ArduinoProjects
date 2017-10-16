#ifndef DoorMutator_h
#define DoorMutator_h

#include "arduino.h"
#include "Logger.h"
#include "StateMutator.h"

class DoorMutator : public StateMutator
{
private:
    void StartOpeningDoor();
    void StartClosingDoor();
    void StopDoor();

    Logger* _logger;    

    const int OPEN_PIN = 5;
    const int CLOSE_PIN = 6;
    const int POWER_PIN = 3; // Uses PWN
    
public:
    DoorMutator(Logger* logger);
    virtual State MutateState(State state);    
};

#endif